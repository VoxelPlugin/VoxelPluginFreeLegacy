// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include <forward_list>


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
	delete Data;
	delete MainOctree;
}



// Called when the game starts or when spawned
void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	bNotCreated = false;

	Data = new VoxelData(Depth);

	MainOctree = new ChunkOctree(FIntVector::ZeroValue, Depth);
	MainOctree->CreateTree(this, FVector(0, 0, 0));
	MainOctree->Update();
}

int AVoxelWorld::GetValue(FIntVector position)
{
	return Data->GetValue(position);
}


void AVoxelWorld::Add(FVector hitPoint, FVector normal, float range, int strength)
{
	ModifyVoxel(hitPoint, normal, range, strength, true);
}

void AVoxelWorld::Remove(FVector hitPoint, FVector normal, float range, int strength)
{
	ModifyVoxel(hitPoint, normal, range, strength, false);
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
						Data->SetValue(position, value);
						ScheduleUpdate(position);
					}
				}
			}
		}
	}
	ApplyUpdate();
}



void AVoxelWorld::Update(FIntVector position)
{
	MainOctree->GetLeaf(position)->Update();

	if (position.X % 16 == 0 && position.X != 0)
	{
		MainOctree->GetLeaf(position - FIntVector(1, 0, 0))->Update();
	}
	if (position.Y % 16 == 0 && position.Y != 0)
	{
		MainOctree->GetLeaf(position - FIntVector(0, 1, 0))->Update();
	}
	if (position.Z % 16 == 0 && position.Z != 0)
	{
		MainOctree->GetLeaf(position - FIntVector(0, 0, 1))->Update();
	}
}

void AVoxelWorld::ScheduleUpdate(FIntVector position)
{
	ChunksToUpdate.AddUnique(MainOctree->GetLeaf(position));

	if (position.X % 16 == 0 && position.X != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(position - FIntVector(1, 0, 0)));
	}
	if (position.Y % 16 == 0 && position.Y != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(position - FIntVector(0, 1, 0)));
	}
	if (position.Z % 16 == 0 && position.Z != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(position - FIntVector(0, 0, 1)));
	}
}

void AVoxelWorld::ApplyUpdate()
{
	for (ChunkOctree*& chunk : ChunksToUpdate)
	{
		chunk->Update();
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

