// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include <forward_list>

DEFINE_LOG_CATEGORY(VoxelWorldLog)
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ UpdateAll"), STAT_UpdateAll, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ ApplyQueuedUpdates"), STAT_ApplyQueuedUpdates, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Add"), STAT_Add, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelWorld ~ Remove"), STAT_Remove, STATGROUP_Voxel);

// Sets default values
AVoxelWorld::AVoxelWorld() : bNotCreated(true)
{
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);

	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(3);
}

AVoxelWorld::~AVoxelWorld()
{
	delete MainOctree;
	delete Data;
}


void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	bNotCreated = false;

	Data = new VoxelData(Depth);
	MainOctree = new ChunkOctree(FIntVector::ZeroValue, Depth);

	UpdateCameraPosition(FVector::ZeroVector);
}

int AVoxelWorld::GetDepthAt(FIntVector position)
{
	if (IsInWorld(position))
	{
		TSharedPtr<ChunkOctree> Chunk = MainOctree->GetChunk(position).Pin();
		if (Chunk.IsValid())
		{
			return Chunk->Depth;
		}
		else
		{
			UE_LOG(VoxelWorldLog, Error, TEXT("Error: Cannot GetDepthAt (%d, %d, %d): Not valid"), position.X, position.Y, position.Z);
			return -1;
		}
	}
	else
	{
		UE_LOG(VoxelWorldLog, Error, TEXT("Error: Cannot GetDepthAt (%d, %d, %d): Not in world"), position.X, position.Y, position.Z);
		return -1;
	}
}

void AVoxelWorld::UpdateCameraPosition(FVector position)
{
	// Reset to avoid references to destroyed chunks
	ChunksToUpdate.Reset();
	// Recreate octree
	MainOctree->CreateTree(this, position);
	// Apply updates added when recreating octree
	ApplyQueuedUpdates(true);
}

bool AVoxelWorld::GlobalIsInWorld(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return IsInWorld(IP);
}

int AVoxelWorld::GlobalGetValue(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return GetValue(IP);
}

FColor AVoxelWorld::GlobalGetColor(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return GetColor(IP);
}

void AVoxelWorld::GlobalSetValue(FVector position, int value)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	SetValue(IP, value);
}

void AVoxelWorld::GlobalSetColor(FVector position, FColor color)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	SetColor(IP, color);
}


int AVoxelWorld::GetValue(FIntVector position)
{
	return Data->GetValue(position);
}

FColor AVoxelWorld::GetColor(FIntVector position)
{
	return Data->GetColor(position);
}

void AVoxelWorld::SetValue(FIntVector position, int value)
{
	Data->SetValue(position, value);
}

void AVoxelWorld::SetColor(FIntVector position, FColor color)
{
	Data->SetColor(position, color);
}

TArray<FVoxelChunkSaveStruct> AVoxelWorld::GetSaveArray()
{
	return Data->GetSaveArray();
}

void AVoxelWorld::LoadFromArray(TArray<FVoxelChunkSaveStruct> saveArray)
{
	Data->LoadFromArray(saveArray);
}


void AVoxelWorld::GlobalUpdate(FVector position, bool async)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	Update(IP, async);
}

void AVoxelWorld::GlobalScheduleUpdate(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	ScheduleUpdate(IP);
}


void AVoxelWorld::GlobalAdd(FVector position, int strength)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	Add(IP, strength);
}

void AVoxelWorld::Add(FIntVector position, int strength)
{
	SCOPE_CYCLE_COUNTER(STAT_Add);
	if (IsInWorld(position))
	{
		Data->SetValue(position, Data->GetValue(position) - strength);
	}
	else
	{
		UE_LOG(VoxelWorldLog, Error, TEXT("Not in world"));
	}
}

void AVoxelWorld::GlobalRemove(FVector position, int strength)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	Remove(IP, strength);
}

void AVoxelWorld::Remove(FIntVector position, int strength)
{
	SCOPE_CYCLE_COUNTER(STAT_Remove);
	if (IsInWorld(position))
	{
		Data->SetValue(position, Data->GetValue(position) + strength);
	}
	else
	{
		UE_LOG(VoxelWorldLog, Error, TEXT("Not in world"));
	}
}


void AVoxelWorld::Update(FIntVector position, bool async)
{
	if (ChunksToUpdate.Num() != 0)
	{
		UE_LOG(VoxelWorldLog, Warning, TEXT("Update called but there are still chunks in queue"));
	}
	ScheduleUpdate(position);
	ApplyQueuedUpdates(async);
}

void AVoxelWorld::ScheduleUpdate(FIntVector position)
{
	int x = position.X + Size() / 2;
	int y = position.Y + Size() / 2;
	int z = position.Z + Size() / 2;

	bool bXIsAtBorder = x % 16 == 0 && x != 0;
	bool bYIsAtBorder = y % 16 == 0 && y != 0;
	bool bZIsAtBorder = z % 16 == 0 && z != 0;

	ScheduleUpdate(MainOctree->GetChunk(position));

	if (bXIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(1, 0, 0)));
	}
	if (bYIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(0, 1, 0)));
	}
	if (bXIsAtBorder && bYIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(1, 1, 0)));
	}
	if (bZIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(0, 0, 1)));
	}
	if (bXIsAtBorder && bZIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(1, 0, 1)));
	}
	if (bYIsAtBorder && bZIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(0, 1, 1)));
	}
	if (bXIsAtBorder && bYIsAtBorder && bZIsAtBorder)
	{
		ScheduleUpdate(MainOctree->GetChunk(position - FIntVector(1, 1, 1)));
	}
}

void AVoxelWorld::ScheduleUpdate(TWeakPtr<ChunkOctree> chunk)
{
	ChunksToUpdate.AddUnique(chunk);
}

void AVoxelWorld::ApplyQueuedUpdates(bool async)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyQueuedUpdates);
	UE_LOG(VoxelWorldLog, Log, TEXT("Updating %d chunks"), ChunksToUpdate.Num());
	for (auto& Chunk : ChunksToUpdate)
	{
		TSharedPtr<ChunkOctree> LockedObserver(Chunk.Pin());

		if (LockedObserver.IsValid())
		{
			LockedObserver->Update(async);
		}
		else
		{
			UE_LOG(VoxelWorldLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	ChunksToUpdate.Reset();
}

void AVoxelWorld::UpdateAll(bool async)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateAll);
	MainOctree->Update(async);
}



bool AVoxelWorld::CanEditChange(const UProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, Depth) || InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AVoxelWorld, VoxelMaterial))
		return ParentVal && bNotCreated;
	else
		return ParentVal;
}



bool AVoxelWorld::IsInWorld(FIntVector position)
{
	return Data->IsInWorld(position);
}

int AVoxelWorld::Size()
{
	return Data->Size();
}

