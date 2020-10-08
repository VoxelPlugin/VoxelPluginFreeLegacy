// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelTexture.h"
#include "VoxelExposedNodes.h"
#include "VoxelTextureSamplerNode.generated.h"

class UTexture2D;

// Texture sampler. Inputs are in the texture dimension, not between 0 and 1
UCLASS(DisplayName = "Texture Sampler", Category = "Texture", meta = (Keywords = "constant parameter"))
class VOXELGRAPH_API UVoxelNode_TextureSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Texture settings")
	UTexture2D* Texture;
	
	UPROPERTY(EditAnywhere, Category = "Texture settings", meta = (ReconstructNode))
	bool bBilinearInterpolation = true;
	
	UPROPERTY(EditAnywhere, Category = "Texture settings")
	EVoxelSamplerMode Mode = EVoxelSamplerMode::Tile;

	UVoxelNode_TextureSampler();

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Texture); }
	//~ End UVoxelExposedNode Interface
};

// Voxel Texture sampler. Inputs are in the texture dimension, not between 0 and 1
// The voxel texture can only be set in BP
// You can create a voxel texture from another graph, or using erosion
UCLASS(DisplayName = "Voxel Texture Sampler", Category = "Texture", meta = (Keywords = "constant parameter"))
class VOXELGRAPH_API UVoxelNode_VoxelTextureSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Texture settings")
	bool bBilinearInterpolation = true;
	
	UPROPERTY(EditAnywhere, Category = "Texture settings")
	EVoxelSamplerMode Mode = EVoxelSamplerMode::Tile;

	// For parameters to work
	UPROPERTY()
	FVoxelFloatTexture Texture;

	UVoxelNode_VoxelTextureSampler();

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Texture); }
	//~ End UVoxelExposedNode Interface
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};