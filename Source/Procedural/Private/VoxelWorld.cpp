// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelWorld.h"
#include "VoxelData.h"
#include "ChunkOctree.h"
#include "Components/CapsuleComponent.h"
#include "EngineGlobals.h"
#include "Engine.h"
#include <forward_list>
#include <tuple>


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

	FVector p = GetActorLocation();
	int x = FMath::RoundToInt(p.X);
	int y = FMath::RoundToInt(p.Y);
	int z = FMath::RoundToInt(p.Z);

	Data = new VoxelData(x, y, z, Depth);

	MainOctree = new ChunkOctree(x, y, z, Depth);
	MainOctree->CreateTree(this, FVector(0, 0, 0));
	MainOctree->Update();
}

signed char AVoxelWorld::GetValue(int x, int y, int z)
{
	return Data->GetValue(x, y, z);
}

void AVoxelWorld::Add(FVector hitPoint, FVector normal, float range)
{
	FVector P = GetTransform().InverseTransformPosition(hitPoint);
	FVector N = GetTransform().InverseTransformVector(normal).GetSafeNormal();

	// Tangent
	FVector T;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	T.X = 1;
	T.Y = 1;
	T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y);
	T.Normalize();

	// Bitangent
	FVector BT = FVector::CrossProduct(T, N).GetSafeNormal();

	TArray<std::tuple<int, int, int>> processedPoints;

	int m = FMath::CeilToInt(range);
	for (int i = -m; i <= m; i++)
	{
		for (int j = -m; j <= m; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				if (FMath::Sqrt(i*i + j*j) < range)
				{
					FVector Q = P + i * T + j * BT + k * N / 2;
					int x = FMath::RoundToInt(Q.X);
					int y = FMath::RoundToInt(Q.Y);
					int z = FMath::RoundToInt(Q.Z);

					if (IsInWorld(x, y, z) && !processedPoints.Contains(std::tuple<int, int, int>(x, y, z)))
					{
						processedPoints.Add(std::tuple<int, int, int>(x, y, z));
						int value = (int)Data->GetValue(x, y, z) - 1;
						Data->SetValue(x, y, z, FMath::Clamp(value, -127, 127));
						ScheduleUpdate(x, y, z);
					}
				}
			}
		}
	}
	ApplyUpdate();
}

void AVoxelWorld::Remove(FVector hitPoint, FVector normal, float range)
{
	FVector P = GetTransform().InverseTransformPosition(hitPoint);
	FVector N = GetTransform().InverseTransformVector(normal).GetSafeNormal();

	// Tangent
	FVector T;
	// N dot T = 0
	// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
	// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y)
	T.X = 1;
	T.Y = 1;
	T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y);
	T.Normalize();

	// Bitangent
	FVector BT = FVector::CrossProduct(T, N).GetSafeNormal();

	TArray<std::tuple<int, int, int>> processedPoints;

	int m = FMath::CeilToInt(range);
	for (int i = -m; i <= m; i++)
	{
		for (int j = -m; j <= m; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				if (FMath::Sqrt(i*i + j*j) < range)
				{
					FVector Q = P + i * T + j * BT - k * N / 2;
					int x = FMath::RoundToInt(Q.X);
					int y = FMath::RoundToInt(Q.Y);
					int z = FMath::RoundToInt(Q.Z);

					if (IsInWorld(x, y, z) && !processedPoints.Contains(std::tuple<int, int, int>(x, y, z)))
					{
						processedPoints.Add(std::tuple<int, int, int>(x, y, z));
						int value = (int)Data->GetValue(x, y, z) + 1;
						Data->SetValue(x, y, z, FMath::Clamp(value, -127, 127));
						ScheduleUpdate(x, y, z);
					}
				}
			}
		}
	}
	ApplyUpdate();
}

void AVoxelWorld::Update(int x, int y, int z)
{
	MainOctree->GetLeaf(x, y, z)->Update();
	if (x % 16 == 0 && x != 0)
	{
		MainOctree->GetLeaf(x - 1, y, z)->Update();
	}
	if (y % 16 == 0 && y != 0)
	{
		MainOctree->GetLeaf(x, y - 1, z)->Update();
	}
	if (z % 16 == 0 && z != 0)
	{
		MainOctree->GetLeaf(x, y, z - 1)->Update();
	}
}

void AVoxelWorld::ScheduleUpdate(int x, int y, int z)
{
	ChunksToUpdate.AddUnique(MainOctree->GetLeaf(x, y, z));
	if (x % 16 == 0 && x != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(x - 1, y, z));
	}
	if (y % 16 == 0 && y != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(x, y - 1, z));
	}
	if (z % 16 == 0 && z != 0)
	{
		ChunksToUpdate.AddUnique(MainOctree->GetLeaf(x, y, z - 1));
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

bool AVoxelWorld::IsInWorld(int x, int y, int z)
{
	return 0 <= x && x < (16 << Depth) && 0 <= y && y < (16 << Depth) && 0 <= z && z < (16 << Depth);
}

