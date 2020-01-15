// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelGlobals.h"
#include "UObject/Interface.h"
#include "VoxelPhysicsPartSpawnerInterface.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class UMaterialInterface;
class FVoxelData;
class AVoxelWorld;

USTRUCT(BlueprintType)
struct FVoxelPositionValueMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial Material = FVoxelMaterial(ForceInit);
};

UINTERFACE(BlueprintType)
class VOXEL_API UVoxelPhysicsPartSpawner : public UInterface
{
	GENERATED_BODY()
};

class VOXEL_API IVoxelPhysicsPartSpawner : public IInterface
{
	GENERATED_BODY()
	
public:
};
