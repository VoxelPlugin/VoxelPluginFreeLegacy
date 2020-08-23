// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "VoxelSpawnerActor.generated.h"

class UVoxelPhysicsRelevancyComponent;
class UStaticMeshComponent;

// Actor that can be spawned by voxel spawners
// Base class: does nothing
UCLASS()
class VOXEL_API AVoxelSpawnerActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel")
	void SetStaticMesh(UStaticMesh* Mesh, const TMap<int32, UMaterialInterface*>& SectionsMaterials, const FBodyInstance& CollisionPresets);

	UFUNCTION(BlueprintNativeEvent, Category = "Voxel")
	void SetInstanceRandom(float Value);

	virtual void SetStaticMesh_Implementation(UStaticMesh* Mesh, const TMap<int32, UMaterialInterface*>& SectionsMaterials, const FBodyInstance& CollisionPresets) {}
	virtual void SetInstanceRandom_Implementation(float Value) {}
};

// Basic voxel actor with a static mesh component
UCLASS()
class VOXEL_API AVoxelMeshSpawnerActor : public AVoxelSpawnerActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UStaticMeshComponent* StaticMeshComponent;

	AVoxelMeshSpawnerActor();

	virtual void SetStaticMesh_Implementation(UStaticMesh* Mesh, const TMap<int32, UMaterialInterface*>& SectionsMaterials, const FBodyInstance& CollisionPresets) override;
	virtual void SetInstanceRandom_Implementation(float Value) override;
};

// Basic voxel actor with a static mesh component and a voxel physics relevancy component: physics will be frozen when outside the voxel world collision range
UCLASS()
class VOXEL_API AVoxelMeshWithPhysicsRelevancySpawnerActor : public AVoxelMeshSpawnerActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UVoxelPhysicsRelevancyComponent* PhysicsRelevancyComponent;

	AVoxelMeshWithPhysicsRelevancySpawnerActor();
};