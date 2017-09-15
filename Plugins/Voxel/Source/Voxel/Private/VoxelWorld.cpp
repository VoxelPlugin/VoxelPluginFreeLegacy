// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"
#include <forward_list>
#include "FlatWorldGenerator.h"

DEFINE_LOG_CATEGORY(VoxelLog)
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateAll"), STAT_UpdateAll, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ ApplyQueuedUpdates"), STAT_ApplyQueuedUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Add"), STAT_Add, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Remove"), STAT_Remove, STATGROUP_Voxel);

// Sets default values
AVoxelWorld::AVoxelWorld() : bDebugMultiplayer(false), bDrawChunksBorders(false), Depth(9), MultiplayerFPS(5), DeletionDelay(0.1f),
LODToleranceZone(0.5), bRebuildBorders(true), PlayerCamera(nullptr), bAutoFindCamera(true), bAutoUpdateCameraPosition(true), bIsCreated(false), TimeSinceSync(0)
{
	PrimaryActorTick.bCanEverTick = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);

	WorldGenerator = TSubclassOf<AVoxelWorldGenerator>(AFlatWorldGenerator::StaticClass());

	bReplicates = true;

	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(24, 64 * 1024);

	LODProfile = UVoxelLODProfile::StaticClass();
}

AVoxelWorld::~AVoxelWorld()
{
	delete ThreadPool;
	delete Data;
}

void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	Load();
}

void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoFindCamera)
	{
		if (PlayerCamera == nullptr)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCameraManager::StaticClass(), FoundActors);

			if (FoundActors.Num() == 0)
			{
				UE_LOG(VoxelLog, Warning, TEXT("No camera found"));
			}
			else if (FoundActors.Num() == 1)
			{
				PlayerCamera = (APlayerCameraManager*)FoundActors[0];
			}
			else
			{
				UE_LOG(VoxelLog, Warning, TEXT("More than one camera found"));
			}
		}
	}

	if (bAutoUpdateCameraPosition && PlayerCamera != nullptr)
	{
		UpdateCameraPosition(PlayerCamera->GetTransform().GetLocation());
	}

	if (bMultiplayer)
	{
		TimeSinceSync += DeltaTime;
		if (TimeSinceSync > 1 / MultiplayerFPS)
		{
			Sync();
			TimeSinceSync = 0;
		}
	}
}

AVoxelChunk* AVoxelWorld::GetChunkAt(FIntVector Position) const
{
	if (IsInWorld(Position) && MainChunkOctree.IsValid())
	{
		TSharedPtr<ChunkOctree> Chunk = MainChunkOctree->GetChunk(Position).Pin();
		if (Chunk.IsValid())
		{
			return Chunk->GetVoxelChunk();
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

FColor AVoxelWorld::GetColor(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		return Data->GetColor(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return FColor::Black;
	}
}

void AVoxelWorld::UpdateCameraPosition(FVector Position)
{
	// Reset to avoid references to destroyed chunks
	QueuedChunks.Reset();
	// Recreate octree
	MainChunkOctree->UpdateCameraPosition(this, Position);
	// Apply updates added when recreating octree
	ApplyQueuedUpdates(true);
}


float AVoxelWorld::GetValue(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		return Data->GetValue(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 0;
	}
}

FVoxelMaterial AVoxelWorld::GetMaterial(FIntVector Position) const
{
	return FVoxelMaterial(GetColor(Position));
}


void AVoxelWorld::SetValue(FIntVector Position, float Value)
{
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Value);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void AVoxelWorld::SetMaterial(FIntVector Position, FVoxelMaterial Material)
{
	if (IsInWorld(Position))
	{
		Data->SetColor(Position, Material.ToFColor());
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}


FVoxelWorldSave AVoxelWorld::GetSave() const
{
	return Data->GetSave();
}

void AVoxelWorld::LoadFromSave(FVoxelWorldSave Save, bool bReset, bool bAsync)
{
	if (Save.Depth == Depth)
	{
		auto ChunksList = Save.GetChunksList();
		Data->LoadAndQueueUpdateFromSave(ChunksList, this, bReset);
		ApplyQueuedUpdates(bAsync);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Current Depth is %d while Save one is %d"), Depth, Save.Depth);
	}
}

void AVoxelWorld::Sync()
{
	std::forward_list<TArray<FVoxelValueDiff>> ValueDiffPacketsList;
	std::forward_list<TArray<FVoxelColorDiff>> ColorDiffPacketsList;

	Data->GetDiffArrays(ValueDiffPacketsList, ColorDiffPacketsList);

	while (!ValueDiffPacketsList.empty() || !ColorDiffPacketsList.empty())
	{
		TArray<FVoxelValueDiff> ValueDiffArray;
		TArray<FVoxelColorDiff> ColorDiffArray;

		if (!ValueDiffPacketsList.empty())
		{
			ValueDiffArray = ValueDiffPacketsList.front();
			ValueDiffPacketsList.pop_front();
		}

		if (!ColorDiffPacketsList.empty())
		{
			ColorDiffArray = ColorDiffPacketsList.front();
			ColorDiffPacketsList.pop_front();
		}

		MulticastLoadArray(ValueDiffArray, ColorDiffArray);
	}
}


FIntVector AVoxelWorld::GlobalToLocal(FVector Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position);
	return FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
}

void AVoxelWorld::Add(FIntVector Position, float Value)
{
	SCOPE_CYCLE_COUNTER(STAT_Add);
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Data->GetValue(Position) + Value);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
	}
}


void AVoxelWorld::Update(FIntVector Position, bool bAsync)
{
	if (QueuedChunks.Num() != 0)
	{
		UE_LOG(VoxelLog, Warning, TEXT("Update called but there are still chunks in queue"));
	}
	QueueUpdate(Position);
	ApplyQueuedUpdates(bAsync);
}

void AVoxelWorld::QueueUpdate(FIntVector Position)
{
	if (!IsInWorld(Position))
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
		return;
	}

	int X = Position.X + Size() / 2;
	int Y = Position.Y + Size() / 2;
	int Z = Position.Z + Size() / 2;

	bool bXIsAtBorder = (X % 16 == 0) && (X != 0);
	bool bYIsAtBorder = (Y % 16 == 0) && (Y != 0);
	bool bZIsAtBorder = (Z % 16 == 0) && (Z != 0);

	QueueUpdate(MainChunkOctree->GetChunk(Position));

	if (bXIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(8, 0, 0)));
	}
	if (bYIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(0, 8, 0)));
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(8, 8, 0)));
	}
	if (bZIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(0, 0, 8)));
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(8, 0, 8)));
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(0, 8, 8)));
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainChunkOctree->GetChunk(Position - FIntVector(8, 8, 8)));
	}
}

void AVoxelWorld::QueueUpdate(TWeakPtr<ChunkOctree> Chunk)
{
	QueuedChunks.Add(Chunk);
}


void AVoxelWorld::ApplyQueuedUpdates(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyQueuedUpdates);
	//UE_LOG(VoxelLog, Log, TEXT("Updating %d chunks"), QueuedChunks.Num());

	for (auto& Chunk : QueuedChunks)
	{
		TSharedPtr<ChunkOctree> LockedObserver(Chunk.Pin());

		if (LockedObserver.IsValid())
		{
			LockedObserver->Update(bAsync);
		}
		else
		{
			UE_LOG(VoxelLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	QueuedChunks.Reset();
}

void AVoxelWorld::UpdateAll(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAll);
	MainChunkOctree->Update(bAsync);
}


void AVoxelWorld::Load()
{
	if (!bIsCreated)
	{
		UE_LOG(VoxelLog, Warning, TEXT("Loading world"));
		bIsCreated = true;

		InstancedWorldGenerator = GetWorld()->SpawnActor<AVoxelWorldGenerator>(WorldGenerator);

		if (InstancedWorldGenerator == nullptr)
		{
			UE_LOG(VoxelLog, Error, TEXT("Invalid world generator"));
			InstancedWorldGenerator = Cast<AVoxelWorldGenerator>(GetWorld()->SpawnActor(AFlatWorldGenerator::StaticClass()));
		}

		InstancedWorldGenerator->SetVoxelWorld(this);

		Data = new VoxelData(Depth, InstancedWorldGenerator, bMultiplayer);
		MainChunkOctree = MakeShareable(new ChunkOctree(FIntVector::ZeroValue, Depth));
	}
}

void AVoxelWorld::Unload()
{
	if (bIsCreated)
	{
		MainChunkOctree->ImmediateDelete();
		MainChunkOctree.Reset();
		delete Data;
		bIsCreated = false;
	}
}



#if WITH_EDITOR
bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, bMultiplayer))
		return ParentVal && !bIsCreated;
	else
		return ParentVal;
}
#endif // WITH_EDITOR


void AVoxelWorld::MulticastLoadArray_Implementation(const TArray<FVoxelValueDiff>& ValueDiffArray, const TArray<FVoxelColorDiff>& ColorDiffArray)
{
	if (!(GetNetMode() < ENetMode::NM_Client))
	{
		if (bDebugMultiplayer)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Loading %d values"), ValueDiffArray.Num()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Loading %d colors"), ColorDiffArray.Num()));
		}

		Data->LoadAndQueueUpdateFromDiffArray(ValueDiffArray, ColorDiffArray, this);
		ApplyQueuedUpdates(false);
	}
}


bool AVoxelWorld::IsInWorld(FIntVector Position) const
{
	return Data->IsInWorld(Position);
}

int AVoxelWorld::Size() const
{
	return Data->Width();
}

float AVoxelWorld::GetDeletionDelay() const
{
	return DeletionDelay;
}

float AVoxelWorld::GetLODToleranceZone() const
{
	return LODToleranceZone;
}

bool AVoxelWorld::GetRebuildBorders() const
{
	return bRebuildBorders;
}

float AVoxelWorld::GetLODAt(float Distance) const
{
	return LODProfile.GetDefaultObject()->GetLODAt(Distance);
}

FQueuedThreadPool* AVoxelWorld::GetThreadPool()
{
	return ThreadPool;
}

VoxelData* AVoxelWorld::GetData()
{
	return Data;
}

bool AVoxelWorld::IsCreated() const
{
	return bIsCreated;
}

TSharedPtr<ChunkOctree> AVoxelWorld::GetChunkOctree() const
{
	return TSharedPtr<ChunkOctree>(MainChunkOctree);
}

TSharedPtr<ValueOctree> AVoxelWorld::GetValueOctree() const
{
	return Data->GetValueOctree();
}

