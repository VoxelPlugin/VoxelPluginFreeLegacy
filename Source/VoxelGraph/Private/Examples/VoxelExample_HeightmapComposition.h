// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_HeightmapComposition.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_HeightmapComposition : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// Depth below the heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Depth"))
	float Depth = 300.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Flip X"))
	bool Flip_X = false;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Flip Y"))
	bool Flip_Y = true;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="heightmap x0 y0"))
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x0_y0 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x0_y0.heightmap_x0_y0"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="heightmap x0 y1"))
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x0_y1 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x0_y1.heightmap_x0_y1"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="heightmap x1 y0"))
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x1_y0 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x1_y0.heightmap_x1_y0"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="heightmap x1 y1"))
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x1_y1 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x1_y1.heightmap_x1_y1"));
	// Size of the heightmaps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Size X"))
	float Size_X = 512.0;
	// Size of the heightmaps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Size Y"))
	float Size_Y = 512.0;
	
	UVoxelExample_HeightmapComposition();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
