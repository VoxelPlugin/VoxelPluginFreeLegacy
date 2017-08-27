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
AVoxelWorld::AVoxelWorld() : Depth(10), MultiplayerFPS(5), DeletionDelay(0.1f), Quality(0.75f), HighResolutionDistanceOffset(25), bRebuildBorders(true),
PlayerCamera(nullptr), bAutoFindCamera(true), bAutoUpdateCameraPosition(true), bNotCreated(true), TimeSinceSync(0)
{
	PrimaryActorTick.bCanEverTick = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);

	WorldGenerator = UFlatWorldGenerator::StaticClass();

	bReplicates = true;
}


void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	bNotCreated = false;

	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(8);

	UObject* WorldGeneratorObject = WorldGenerator->GetDefaultObject();

	if (!WorldGeneratorObject->GetClass()->ImplementsInterface(UVoxelWorldGenerator::StaticClass()))
	{
		UE_LOG(VoxelLog, Error, TEXT("Invalid world generator"));
		WorldGeneratorObject = UFlatWorldGenerator::StaticClass()->GetDefaultObject();
	}

	WorldGeneratorInterface.SetInterface(Cast<IInterface>(WorldGeneratorObject));
	WorldGeneratorInterface.SetObject(WorldGeneratorObject);

	Data = MakeShareable(new VoxelData(Depth, WorldGeneratorInterface, bMultiplayer));
	MainOctree = MakeShareable(new ChunkOctree(FIntVector::ZeroValue, Depth));

	UpdateCameraPosition(FVector::ZeroVector);
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
	if (IsInWorld(Position))
	{
		TSharedPtr<ChunkOctree> Chunk = MainOctree->GetChunk(Position).Pin();
		check(Chunk.IsValid());
		return Chunk->GetVoxelChunk();
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Error: Cannot GetChunkAt (%d, %d, %d): Not in world"), Position.X, Position.Y, Position.Z);
		return nullptr;
	}
}

void AVoxelWorld::UpdateCameraPosition(FVector Position)
{
	// Reset to avoid references to destroyed chunks
	QueuedChunks.Reset();
	// Recreate octree
	MainOctree->UpdateCameraPosition(this, Position);
	// Apply updates added when recreating octree
	ApplyQueuedUpdates(true);
}


float AVoxelWorld::GetValue(FIntVector Position) const
{
	return Data->GetValue(Position);
}

FColor AVoxelWorld::GetColor(FIntVector Position) const
{
	return Data->GetColor(Position);
}

void AVoxelWorld::SetValue(FIntVector Position, float Value) const
{
	Data->SetValue(Position, Value);
}

void AVoxelWorld::SetColor(FIntVector Position, FColor Color) const
{
	Data->SetColor(Position, Color);
}

FVoxelWorldSave AVoxelWorld::GetSave() const
{
	return FVoxelWorldSave(Depth, Data->GetSaveArray());
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

void AVoxelWorld::Add(FIntVector Position, float Value) const
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

	QueueUpdate(MainOctree->GetChunk(Position));

	if (bXIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 0, 0)));
	}
	if (bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 8, 0)));
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 8, 0)));
	}
	if (bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 0, 8)));
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 0, 8)));
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 8, 8)));
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(8, 8, 8)));
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

void AVoxelWorld::UpdateAll(bool bAsync) const
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAll);
	MainOctree->Update(bAsync);
}


#if WITH_EDITOR
bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, bMultiplayer))
		return ParentVal && bNotCreated;
	else
		return ParentVal;
}
#endif


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
	return Data->Size();
}

float AVoxelWorld::GetDeletionDelay() const
{
	return DeletionDelay;
}

float AVoxelWorld::GetQuality() const
{
	return Quality;
}

float AVoxelWorld::GetHighResolutionDistanceOffset() const
{
	return HighResolutionDistanceOffset;
}

bool AVoxelWorld::GetRebuildBorders() const
{
	return bRebuildBorders;
}

TSharedPtr<ChunkOctree> AVoxelWorld::GetChunkOctree() const
{
	return TSharedPtr<ChunkOctree>(MainOctree);
}

TSharedPtr<ValueOctree> AVoxelWorld::GetValueOctree() const
{
	return Data->GetValueOctree();
}

