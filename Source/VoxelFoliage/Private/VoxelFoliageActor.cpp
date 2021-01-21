// Copyright 2021 Phyronnaz

#include "VoxelFoliageActor.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelComponents/VoxelPhysicsRelevancyComponent.h"
#include "UObject/ConstructorHelpers.h"

AVoxelFoliageActor::AVoxelFoliageActor()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	PhysicsRelevancyComponent = CreateDefaultSubobject<UVoxelPhysicsRelevancyComponent>("VoxelPhysicsRelevancyComponent");

	RootComponent = StaticMeshComponent;

	// We set a default value so that bSimulatePhysics can be changed
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	StaticMeshComponent->SetStaticMesh(SphereMeshFinder.Object);
}