// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include <forward_list>

DEFINE_LOG_CATEGORY(VoxelWorldLog)

// Sets default values
AVoxelWorld::AVoxelWorld() : bNotCreated(true)
{
	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	SetActorScale3D(100 * FVector::OneVector);
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
	ApplyQueuedUpdates();
}

bool AVoxelWorld::IsInWorld(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return IsInWorld(IP);
}

int AVoxelWorld::GetValue(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return GetValue(IP);
}

FColor AVoxelWorld::GetColor(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	return GetColor(IP);
}

void AVoxelWorld::SetValue(FVector position, int value)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	SetValue(IP, value);
}

void AVoxelWorld::SetColor(FVector position, FColor color)
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


void AVoxelWorld::Add(FVector hitPoint, FVector normal, float range, int strength)
{
	ModifyVoxel(hitPoint, normal, range, strength, true);
}

void AVoxelWorld::Remove(FVector hitPoint, FVector normal, float range, int strength)
{
	ModifyVoxel(hitPoint, normal, range, strength, false);
}

void AVoxelWorld::Update(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	Update(IP);
}

void AVoxelWorld::ScheduleUpdate(FVector position)
{
	FVector P = GetTransform().InverseTransformPosition(position);
	FIntVector IP = FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
	ScheduleUpdate(IP);
}

void AVoxelWorld::ModifyVoxel(FVector hitPoint, FVector normal, float range, int strength, bool add)
{
	FVector Position = GetTransform().InverseTransformPosition(hitPoint);
	FVector Normal = GetTransform().InverseTransformVector(normal).GetSafeNormal();

	// Compute tangent
	FVector Tangent;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	Tangent.X = 1;
	Tangent.Y = 1;
	Tangent.Z = -1 / Normal.Z * (Normal.X * Tangent.X + Normal.Y * Tangent.Y);
	Tangent.Normalize();

	// Compute bitangent
	FVector Bitangent = FVector::CrossProduct(Tangent, Normal).GetSafeNormal();

	TArray<FIntVector> ProcessedPoints;

	int m = FMath::CeilToInt(range);
	for (int i = -m; i <= m; i++)
	{
		for (int j = -m; j <= m; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				if (i*i + j*j < range*range)
				{
					FVector Q = Position + i * Tangent + j * Bitangent + k * Normal / 2 * (add ? 1 : -1);
					int x = FMath::RoundToInt(Q.X);
					int y = FMath::RoundToInt(Q.Y);
					int z = FMath::RoundToInt(Q.Z);

					FIntVector position = FIntVector(x, y, z);

					if (IsInWorld(position) && !ProcessedPoints.Contains(position))
					{
						ProcessedPoints.Add(position);
						int value = (int)Data->GetValue(position) + (add ? 1 : -1) * strength;
						SetValue(position, value);
						ScheduleUpdate(position);
					}
				}
			}
		}
	}
	ApplyQueuedUpdates();
}



void AVoxelWorld::Update(FIntVector position)
{
	if (ChunksToUpdate.Num() != 0)
	{
		UE_LOG(VoxelWorldLog, Warning, TEXT("Update called but there are still chunks in queue"));
	}
	ScheduleUpdate(position);
	ApplyQueuedUpdates();
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

void AVoxelWorld::ApplyQueuedUpdates()
{
	for (auto& Chunk : ChunksToUpdate)
	{
		TSharedPtr<ChunkOctree> LockedObserver(Chunk.Pin());

		if (LockedObserver.IsValid())
		{
			LockedObserver->Update();
		}
		else
		{
			UE_LOG(VoxelWorldLog, Warning, TEXT("Invalid chunk in queue"));
		}
	}
	ChunksToUpdate.Reset();
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

