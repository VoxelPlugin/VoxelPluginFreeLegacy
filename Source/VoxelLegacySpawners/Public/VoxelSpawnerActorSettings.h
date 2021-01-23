// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelSpawnerActorSettings.generated.h"

class AVoxelFoliageActor;

USTRUCT(BlueprintType)
struct VOXELLEGACYSPAWNERS_API FVoxelSpawnerActorSettings
{
	GENERATED_BODY()

	FVoxelSpawnerActorSettings()
	{
		BodyInstance.SetCollisionProfileName("BlockAll");
	}

public:
	// Actor to spawn to replace the instanced mesh. After spawn, the SetStaticMesh event will be called on the actor with Mesh as argument
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	TSubclassOf<AVoxelFoliageActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings", meta = (ShowOnlyInnerProperties))
	FBodyInstance BodyInstance;
	
	// Set the lifespan of this actor. When it expires the object will be destroyed.
	// Set to 0 to disable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings", meta = (ClampMin = 0))
	float Lifespan = 5.f;
};