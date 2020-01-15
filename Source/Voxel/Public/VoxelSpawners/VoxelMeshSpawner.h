// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSpawners/VoxelBasicSpawner.h"
#include "VoxelSpawners/VoxelActor.h"
#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "Templates/SubclassOf.h"
#include "Engine/EngineTypes.h"
#include "VoxelMeshSpawner.generated.h"

class FVoxelConstDataAccelerator;
class FVoxelMeshSpawnerProxy;
class FVoxelMeshSpawnerGroupProxy;
class UStaticMesh;
class UVoxelMeshSpawnerBase;
class UVoxelMeshSpawnerGroup;
class UVoxelHierarchicalInstancedStaticMeshComponent;


UCLASS(Abstract)
class VOXEL_API UVoxelMeshSpawnerBase : public UVoxelBasicSpawner
{
	GENERATED_BODY()

public:
	// Actor to spawn when enabling physics. After spawn, the SetStaticMesh event will be called on the actor with Mesh as argument
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TSubclassOf<AVoxelSpawnerActor> ActorTemplate = AVoxelSpawnerActorWithStaticMeshAndAutoDisable::StaticClass();

	// Will always spawn an actor instead of an instanced mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	bool bAlwaysSpawnActor = false;

public:
	// If true, the voxel material will be sent through PerInstanceRandom, allowing to eg set the instance color to the material color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Settings")
	bool bSendVoxelMaterialThroughInstanceRandom = false;
	
	UPROPERTY(EditAnywhere, Category = "Instance Settings", meta = (ShowOnlyInnerProperties))
	FVoxelInstancedMeshSettings InstancedMeshSettings;
	
public:
	// In cm. Increase this if your foliage is enabling physics too soon
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FVector FloatingDetectionOffset = FVector(0, 0, -10);

};

UCLASS()
class VOXEL_API UVoxelMeshSpawner : public UVoxelMeshSpawnerBase
{
	GENERATED_BODY()

public:
	// Mesh to spawn. Can be left to null if AlwaysSpawnActor is true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* Mesh = nullptr;
	
};

UCLASS()
class VOXEL_API UVoxelMeshSpawnerGroup : public UVoxelMeshSpawnerBase
{
	GENERATED_BODY()

public:
	// Meshes to spawn. Can be left to null if AlwaysSpawnActor is true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TArray<UStaticMesh*> Meshes;
	
};