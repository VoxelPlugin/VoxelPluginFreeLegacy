// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelActor.generated.h"

class UVoxelAutoDisableComponent;
class UStaticMeshComponent;

// Actor that can be spawned by voxel spawners
UCLASS()
class VOXEL_API AVoxelSpawnerActor : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Voxel")
	void SetStaticMesh(UStaticMesh* Mesh);

	UFUNCTION(BlueprintNativeEvent, Category = "Voxel")
	void SetInstanceRandom(float Value);

	virtual void SetStaticMesh_Implementation(UStaticMesh* Mesh) {}
	virtual void SetInstanceRandom_Implementation(float Value) {}
};

// Basic voxel actor with a static mesh component
UCLASS()
class VOXEL_API AVoxelSpawnerActorWithStaticMesh : public AVoxelSpawnerActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UStaticMeshComponent* StaticMeshComponent;

	AVoxelSpawnerActorWithStaticMesh();

	virtual void SetStaticMesh_Implementation(UStaticMesh* Mesh) override;
	virtual void SetInstanceRandom_Implementation(float Value) override;
};

// Basic voxel actor with a static mesh component and a voxel auto disable component: physics will be frozen when outside the voxel world collision range
UCLASS()
class VOXEL_API AVoxelSpawnerActorWithStaticMeshAndAutoDisable : public AVoxelSpawnerActorWithStaticMesh
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Voxel")
	UVoxelAutoDisableComponent* AutoDisableComponent;

	AVoxelSpawnerActorWithStaticMeshAndAutoDisable();
};