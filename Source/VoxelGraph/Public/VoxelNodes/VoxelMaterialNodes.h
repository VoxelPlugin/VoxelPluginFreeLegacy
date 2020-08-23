// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelMaterialNodes.generated.h"

UCLASS(Abstract, Category = "Math|Material")
class VOXELGRAPH_API UVoxelMaterialNode : public UVoxelNodeHelper
{
	GENERATED_BODY()
};

// Get the material color. Outputs are between 0 and 1
UCLASS(DisplayName = "Get Color")
class VOXELGRAPH_API UVoxelNode_GetColor : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetColor();

	// Note: Materials aren't supported by pure nodes
};

// Get the index of the material. Output is between 0 and 255
UCLASS(DisplayName = "Get Single Index")
class VOXELGRAPH_API UVoxelNode_GetIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetIndex();
};

// Read a UV channel from a material.
UCLASS(DisplayName = "Get UV Channel")
class VOXELGRAPH_API UVoxelNode_GetUVChannel : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetUVChannel();
};