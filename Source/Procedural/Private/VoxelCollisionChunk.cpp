// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelCollisionChunk.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"


AVoxelCollisionChunk::AVoxelCollisionChunk() : TimeBeforeCollisionCooking(0)
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionMesh = CreateDefaultSubobject<URuntimeMeshComponent>(FName("CollisionMesh"));
	CollisionMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RootComponent = CollisionMesh;
}

void AVoxelCollisionChunk::BeginPlay()
{
	Super::BeginPlay();

}

void AVoxelCollisionChunk::Tick(float DeltaTime)
{
	check(VoxelChunk);
	Super::Tick(DeltaTime);
	TimeBeforeCollisionCooking -= DeltaTime;

	if (VoxelChunk->bCollisionDirty && TimeBeforeCollisionCooking < 0)
	{
		VoxelChunk->Update(CollisionMesh, true);
		CollisionMesh->SetMeshSectionVisible(0, false);
		VoxelChunk->bCollisionDirty = false;
		TimeBeforeCollisionCooking = 1 / VoxelChunk->World->CollisionFPS;
	}
}

