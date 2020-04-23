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

// Make a material from a color
UCLASS(DisplayName = "Make Material From Color")
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromColor : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_MakeMaterialFromColor();
};

// Get the index of the material. Output is between 0 and 255
UCLASS(DisplayName = "Get Single Index")
class VOXELGRAPH_API UVoxelNode_GetIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetIndex();
};

// Make a material from a single index
// DataA/B/C can be used in the material using the GetSingleIndexData material function
// DataA/B/C are between 0 and 1
UCLASS(DisplayName = "Make Material From Single Index")
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromSingleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_MakeMaterialFromSingleIndex();
};

// Get the double index of the material. Indices are between 0 and 255, blend between 0 and 1
UCLASS(DisplayName = "Get Double Index")
class VOXELGRAPH_API UVoxelNode_GetDoubleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetDoubleIndex();
};

// Make a material from a specified double index
// Data can be used in the material using the GetDoubleIndexData material function
// Data is between 0 and 1
UCLASS(DisplayName = "Make Material From Double Index")
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromDoubleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_MakeMaterialFromDoubleIndex();
};

// Read a UV channel from a material.
UCLASS(DisplayName = "Get UV Channel")
class VOXELGRAPH_API UVoxelNode_GetUVChannel : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetUVChannel();
};

// Make a double index material from a list of indices and alphas
// Data can be used in the material using the GetDoubleIndexData material function
// Data is between 0 and 1
UCLASS(DisplayName = "Create Double Index Material")
class VOXELGRAPH_API UVoxelNode_CreateDoubleIndexMaterial : public UVoxelMaterialNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_CreateDoubleIndexMaterial();

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	//~ End UVoxelNode Interface
};