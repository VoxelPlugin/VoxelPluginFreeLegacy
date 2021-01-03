// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodes/VoxelExposedNodes.h"
#include "VoxelNodes/VoxelNodeHelper.h"
#include "VoxelNodes/VoxelNodeHelperMacros.h"
#include "VoxelFoliage/VoxelFoliageBiome.h"
#include "VoxelFoliageNodes.generated.h"

class UMaterialInterface;

// Get the strength of a material index at the current position. Used for foliage
UCLASS(DisplayName = "Sample Foliage Material Index", Category = "Foliage")
class VOXELGRAPH_API UVoxelNode_SampleFoliageMaterialIndex : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_SampleFoliageMaterialIndex();
};

// Get the strength of a material index at the current position. Used for foliage
UCLASS(DisplayName = "Get Biome Index", Category = "Foliage")
class VOXELGRAPH_API UVoxelNode_GetBiomeIndex : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	GENERATED_EXPOSED_VOXELNODE_BODY(Biome)

public:
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (NonNull))
	UVoxelFoliageBiome* Biome;

	UVoxelNode_GetBiomeIndex();

	virtual FText GetTitle() const override;
};