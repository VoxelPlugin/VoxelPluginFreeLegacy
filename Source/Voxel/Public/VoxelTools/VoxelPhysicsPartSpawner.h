// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelPhysicsPartSpawnerInterface.h"
#include "VoxelPhysicsPartSpawner.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class FVoxelData;
class AVoxelWorld;

UCLASS(BlueprintType)
class VOXEL_API UVoxelPhysicsPartSpawnerResult_VoxelWorlds : public UObject, public IVoxelPhysicsPartSpawnerResult
{
	GENERATED_BODY()

public:
	// The voxel world representing this part
	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	AVoxelWorld* VoxelWorld;
};

// Will spawn a voxel world per part
// The voxel worlds are return in the Results
// You can configure each voxel world by binding ConfigureVoxelWorld: this callback will be run before creating the voxel worlds
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
	
};

///////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType)
class VOXEL_API UVoxelPhysicsPartSpawnerResult_Cubes : public UObject, public IVoxelPhysicsPartSpawnerResult
{
	GENERATED_BODY()
		
public:
	// The cubes for this part
	UPROPERTY(BlueprintReadOnly, Category = "Voxel")
	TArray<AStaticMeshActor*> Cubes;
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

	// Spawn probability for each cube. Use this to reduce the amount of cubes spawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float SpawnProbability = 1.f;
	
};

///////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType)
class VOXEL_API UVoxelPhysicsPartSpawnerResult_GetVoxels : public UObject, public IVoxelPhysicsPartSpawnerResult
{
	GENERATED_BODY()
		
public:
	// The voxels inside this part
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<FVoxelPositionValueMaterial> Voxels;
};

// The data is accessible immediately in the result
UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelPhysicsPartSpawner_GetVoxels : public UObject, public IVoxelPhysicsPartSpawner
{
	GENERATED_BODY()

};