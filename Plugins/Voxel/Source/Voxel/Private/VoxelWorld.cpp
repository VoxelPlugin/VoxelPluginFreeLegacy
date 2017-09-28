// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"
#include <forward_list>
#include "FlatWorldGenerator.h"
#include "VoxelInvokerComponent.h"

DEFINE_LOG_CATEGORY(VoxelLog)
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateAll"), STAT_UpdateAll, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Add"), STAT_Add, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateLOD"), STAT_UpdateLOD, STATGROUP_Voxel);

// Sets default values
AVoxelWorld::AVoxelWorld() : bDebugMultiplayer(false), bDrawChunksBorders(false), Depth(9), MultiplayerFPS(5), DeletionDelay(0.1f), bRebuildBorders(true), bIsCreated(false), TimeSinceSync(0), GrassFPS(5)
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

	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateLOD);
		MainChunkOctree->UpdateLOD(this, VoxelInvokerComponents);
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

	ApplyQueuedUpdates();
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
		UE_LOG(VoxelLog, Error, TEXT("Get color: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return FColor::Black;
	}
}


float AVoxelWorld::GetValue(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		return Data->GetValue(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Get value: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
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
		UE_LOG(VoxelLog, Error, TEXT("Get material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
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
		UE_LOG(VoxelLog, Error, TEXT("Set material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}


FVoxelWorldSave AVoxelWorld::GetSave() const
{
	return Data->GetSave();
}

void AVoxelWorld::LoadFromSave(FVoxelWorldSave Save, bool bReset)
{
	if (Save.Depth == Depth)
	{
		auto ChunksList = Save.GetChunksList();
		Data->LoadAndQueueUpdateFromSave(ChunksList, this, bReset);
		ApplyQueuedUpdates();
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

		if (!ValueDiffPacketsList.empty())
		{
			MulticastSyncValues(ValueDiffPacketsList.front());
			ValueDiffPacketsList.pop_front();
		}

		if (!ColorDiffPacketsList.empty())
		{
			MulticastSyncColors(ColorDiffPacketsList.front());
			ColorDiffPacketsList.pop_front();
		}
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
		UE_LOG(VoxelLog, Error, TEXT("Add: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void AVoxelWorld::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	VoxelInvokerComponents.push_front(Invoker);
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
	return ParentVal;

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial)
		|| InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, bMultiplayer))
		return ParentVal && !bIsCreated;
	else
		return ParentVal;
}
#endif // WITH_EDITOR


void AVoxelWorld::MulticastSyncValues_Implementation(const TArray<FVoxelValueDiff>& ValueDiffArray)
{
	if (!(GetNetMode() < ENetMode::NM_Client))
	{
		if (bDebugMultiplayer)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Loading %d values"), ValueDiffArray.Num()));
		}

		Data->LoadAndQueueUpdateFromDiffArray(ValueDiffArray, TArray<FVoxelColorDiff>(), this);
	}
}

void AVoxelWorld::MulticastSyncColors_Implementation(const TArray<FVoxelColorDiff>& ColorDiffArray)
{
	if (!(GetNetMode() < ENetMode::NM_Client))
	{
		if (bDebugMultiplayer)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Loading %d colors"), ColorDiffArray.Num()));
		}

		Data->LoadAndQueueUpdateFromDiffArray(TArray<FVoxelValueDiff>(), ColorDiffArray, this);
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

bool AVoxelWorld::GetRebuildBorders() const
{
	return bRebuildBorders;
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

