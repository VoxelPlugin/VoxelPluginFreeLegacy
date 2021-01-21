// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelFoliageActor.generated.h"

class UVoxelPhysicsRelevancyComponent;
class UStaticMeshComponent;

UCLASS()
class VOXELFOLIAGE_API AVoxelFoliageActor : public AActor
{
	GENERATED_BODY()

public:
	// If true, will set the StaticMeshComponent mesh (and materials) based on the foliage mesh
	UPROPERTY(EditDefaultsOnly, Category = "Voxel Foliage Actor")
	bool bAutomaticallySetMesh = true;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Foliage Actor")
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel Foliage Actor")
	UVoxelPhysicsRelevancyComponent* PhysicsRelevancyComponent;

	AVoxelFoliageActor();
};