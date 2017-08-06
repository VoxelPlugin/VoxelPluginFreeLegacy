// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include <forward_list>

DEFINE_LOG_CATEGORY(VoxelLog)
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateAll"), STAT_UpdateAll, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ ApplyQueuedUpdates"), STAT_ApplyQueuedUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Add"), STAT_Add, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Remove"), STAT_Remove, STATGROUP_Voxel);

// Sets default values
AVoxelWorld::AVoxelWorld() : bNotCreated(true), Quality(1), DeletionDelay(0.5f), WorldGenerator(NULL)
{
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);

	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(8);
}


void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	bNotCreated = false;

	WorldGeneratorInstance = WorldGenerator.GetDefaultObject();
	if (WorldGeneratorInstance == nullptr)
	{
		NewObject<UVoxelWorldGenerator>(WorldGeneratorInstance);
	}

	Data = MakeShareable(new VoxelData(Depth, WorldGeneratorInstance));
	MainOctree = MakeShareable(new ChunkOctree(FIntVector::ZeroValue, Depth));

	UpdateCameraPosition(FVector::ZeroVector);
}

AVoxelChunk* AVoxelWorld::GetChunkAt(FIntVector Position)
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
	ChunksToUpdate.Reset();
	// Recreate octree
	MainOctree->CreateTree(this, Position);
	// Apply updates added when recreating octree
	ApplyQueuedUpdates(true);
}


int AVoxelWorld::GetValue(FIntVector Position)
{
	return Data->GetValue(Position);
}

FColor AVoxelWorld::GetColor(FIntVector Position)
{
	return Data->GetColor(Position);
}

void AVoxelWorld::SetValue(FIntVector Position, int Value)
{
	Data->SetValue(Position, Value);
}

void AVoxelWorld::SetColor(FIntVector Position, FColor Color)
{
	Data->SetColor(Position, Color);
}

TArray<FVoxelChunkSaveStruct> AVoxelWorld::GetSaveArray()
{
	return Data->GetSaveArray();
}

void AVoxelWorld::LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray)
{
	Data->LoadFromArray(SaveArray);
}


FIntVector AVoxelWorld::GlobalToLocal(FVector Position)
{
	FVector P = GetTransform().InverseTransformPosition(Position);
	return FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
}

void AVoxelWorld::Add(FIntVector Position, int Strength)
{
	SCOPE_CYCLE_COUNTER(STAT_Add);
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Data->GetValue(Position) - Strength);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
	}
}

void AVoxelWorld::Remove(FIntVector Position, int Strength)
{
	SCOPE_CYCLE_COUNTER(STAT_Remove);
	if (IsInWorld(Position))
	{
		Data->SetValue(Position, Data->GetValue(Position) + Strength);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Not in world"));
	}
}


void AVoxelWorld::Update(FIntVector Position, bool bAsync)
{
	if (ChunksToUpdate.Num() != 0)
	{
		UE_LOG(VoxelLog, Warning, TEXT("Update called but there are still chunks in queue"));
	}
	QueueUpdate(Position);
	ApplyQueuedUpdates(bAsync);
}

void AVoxelWorld::QueueUpdate(FIntVector Position)
{
	int X = Position.X + Size() / 2;
	int Y = Position.Y + Size() / 2;
	int Z = Position.Z + Size() / 2;

	bool bXIsAtBorder = X % 16 == 0 && X != 0;
	bool bYIsAtBorder = Y % 16 == 0 && Y != 0;
	bool bZIsAtBorder = Z % 16 == 0 && Z != 0;

	QueueUpdate(MainOctree->GetChunk(Position));

	if (bXIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(1, 0, 0)));
	}
	if (bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 1, 0)));
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(1, 1, 0)));
	}
	if (bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 0, 1)));
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(1, 0, 1)));
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(0, 1, 1)));
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		QueueUpdate(MainOctree->GetChunk(Position - FIntVector(1, 1, 1)));
	}
}

void AVoxelWorld::QueueUpdate(TWeakPtr<ChunkOctree> Chunk)
{
	ChunksToUpdate.AddUnique(Chunk);
}

void AVoxelWorld::ApplyQueuedUpdates(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyQueuedUpdates);
	//UE_LOG(VoxelLog, Log, TEXT("Updating %d chunks"), ChunksToUpdate.Num());

	for (auto& Chunk : ChunksToUpdate)
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
	ChunksToUpdate.Reset();
}

void AVoxelWorld::UpdateAll(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAll);
	MainOctree->Update(bAsync);
}



bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth) || InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial))
		return ParentVal && bNotCreated;
	else
		return ParentVal;
}



bool AVoxelWorld::IsInWorld(FIntVector Position)
{
	return Data->IsInWorld(Position);
}

int AVoxelWorld::Size()
{
	return Data->Size();
}

float AVoxelWorld::GetDeletionDelay()
{
	return DeletionDelay;
}

float AVoxelWorld::GetQuality()
{
	return Quality;
}

bool AVoxelWorld::GetRebuildBorders()
{
	return bRebuildBorders;
}

