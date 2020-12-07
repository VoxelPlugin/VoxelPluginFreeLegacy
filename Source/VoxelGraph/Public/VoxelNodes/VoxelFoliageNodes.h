// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodes/VoxelNodeHelper.h"
#include "VoxelNodes/VoxelNodeHelperMacros.h"
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