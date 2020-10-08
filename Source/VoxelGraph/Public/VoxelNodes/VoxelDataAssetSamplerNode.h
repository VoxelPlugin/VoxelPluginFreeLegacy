// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelExposedNodes.h"
#include "VoxelDataAssetSamplerNode.generated.h"

class UVoxelDataAsset;

// Voxel data asset sampler
UCLASS(DisplayName = "Data Asset Sampler", Category = "Heightmap")
class VOXELGRAPH_API UVoxelNode_DataAssetSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (NonNull))
	UVoxelDataAsset* Asset;
	
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ReconstructNode))
	bool bBilinearInterpolation = true;
	
	UVoxelNode_DataAssetSampler();

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Asset); }
	//~ End UVoxelExposedNode Interface
};