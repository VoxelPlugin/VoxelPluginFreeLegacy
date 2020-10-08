// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelEnums.h"
#include "VoxelHeightmapSamplerNode.generated.h"

class UVoxelHeightmapAssetFloat;
class UVoxelHeightmapAssetUINT16;

// Heightmap sampler
UCLASS(DisplayName = "Heightmap Sampler", Category = "Heightmap")
class VOXELGRAPH_API UVoxelNode_HeightmapSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY()
	bool bFloatHeightmap = false;

	UPROPERTY(EditAnywhere, Category = "Heightmap settings", meta = (DisplayName = "Heightmap (float)", EditCondition = "bFloatHeightmap"))
	UVoxelHeightmapAssetFloat* HeightmapFloat;

	UPROPERTY(EditAnywhere, Category = "Heightmap settings", meta = (DisplayName = "Heightmap (uint16)", EditCondition = "!bFloatHeightmap"))
	UVoxelHeightmapAssetUINT16* HeightmapUINT16;
	
	UPROPERTY(EditAnywhere, Category = "Heightmap settings")
	EVoxelSamplerMode SamplerType = EVoxelSamplerMode::Tile;

	// If true, the heightmap will be centered
	UPROPERTY(EditAnywhere, Category = "Heightmap settings")
	bool bCenter = false;
	
	UVoxelNode_HeightmapSampler();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return bFloatHeightmap ? GET_OWN_MEMBER_NAME(HeightmapFloat) : GET_OWN_MEMBER_NAME(HeightmapUINT16); }
	//~ End UVoxelExposedNode Interface
};