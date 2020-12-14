// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelFoliageActor.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelComponents/VoxelPhysicsRelevancyComponent.h"

AVoxelFoliageActor::AVoxelFoliageActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	PhysicsRelevancyComponent = CreateDefaultSubobject<UVoxelPhysicsRelevancyComponent>("VoxelPhysicsRelevancyComponent");

	RootComponent = StaticMeshComponent;
}