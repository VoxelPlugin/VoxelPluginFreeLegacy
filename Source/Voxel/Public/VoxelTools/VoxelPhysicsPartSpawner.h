// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "Templates/SubclassOf.h"
#include "VoxelPhysicsPartSpawnerInterface.h"
#include "VoxelPhysicsPartSpawner.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class FVoxelData;
class AVoxelWorld;

// Will spawn a voxel world per part
// The voxel worlds are stored in the VoxelWorlds array
// You can configure each voxel world by binding ConfigureVoxelWorld
// The voxel worlds will be created when the initial world update will be done
// By default this will enable SimulatePhysics on the new world, and set the CollisionTraceFlag to SimpleAndComplex
UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelPhysicsPartSpawner_VoxelWorlds : public UObject, public IVoxelPhysicsPartSpawner
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureVoxelWorld, AVoxelWorld*, VoxelWorld);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FConfigureVoxelWorld ConfigureVoxelWorld;

	// All the voxel world properties will be overriden! Use this for events, and use ConfigureVoxelWorld if you want to set properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TSubclassOf<AVoxelWorld> VoxelWorldClass;

	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	TArray<AVoxelWorld*> VoxelWorlds;
	
};

// Will spawn a cube actor for each floating voxel
// Can get the spawned actors in Cubes
// The actors will have physics enabled once the voxel world will be updated (to avoid spawning cubes inside the world)
UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelPhysicsPartSpawner_Cubes : public UObject, public IVoxelPhysicsPartSpawner
{
	GENERATED_BODY()

public:
	UVoxelPhysicsPartSpawner_Cubes();
	
	// Same material as the voxel world, but instead of a vertex color input use a vector parameter named VertexColor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UStaticMesh* CubeMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	TArray<AStaticMeshActor*> Cubes;
	
};

USTRUCT(BlueprintType)
struct FVoxelPositionValueMaterialArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<FVoxelPositionValueMaterial> Voxels;
};

// The data is accessible immediately in the Voxels array of the spawner
UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelPhysicsPartSpawner_GetVoxels : public UObject, public IVoxelPhysicsPartSpawner
{
	GENERATED_BODY()

public:
	// Array of array: parts -> voxels inside part
	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	TArray<FVoxelPositionValueMaterialArray> Voxels;
	
};