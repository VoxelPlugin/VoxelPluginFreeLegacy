// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelLODMaterials.generated.h"

class UMaterialInterface;
class UVoxelMaterialCollectionBase;

USTRUCT(BlueprintType)
struct FVoxelLODMaterialsBase
{
	GENERATED_BODY()

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26))
	int32 StartLOD = 0;

	// Inclusive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 26, UIMin = 0, UIMax = 26))
	int32 EndLOD = 0;
};

USTRUCT(BlueprintType)
struct FVoxelLODMaterials : public FVoxelLODMaterialsBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Material = nullptr;
};

USTRUCT(BlueprintType)
struct FVoxelLODMaterialCollections : public FVoxelLODMaterialsBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UVoxelMaterialCollectionBase* MaterialCollection = nullptr;
};