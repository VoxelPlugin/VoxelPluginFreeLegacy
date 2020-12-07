// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodes/VoxelExposedNodes.h"
#include "VoxelGetLandscapeCollectionIndexNode.generated.h"

class UMaterialInterface;

// Retrieve the index of a material function or a material instance in the voxel world material collection
UCLASS(DisplayName = "Get Landscape Collection Index", Category = "Material")
class VOXELGRAPH_API UVoxelNode_GetLandscapeCollectionIndex : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UPROPERTY(EditAnywhere, Category = "Config", meta = (ReconstructNode))
	FName LayerName = "Layer";

	UVoxelNode_GetLandscapeCollectionIndex();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelAssetPickerNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(LayerName); }
	//~ End UVoxelAssetPickerNode Interface
};