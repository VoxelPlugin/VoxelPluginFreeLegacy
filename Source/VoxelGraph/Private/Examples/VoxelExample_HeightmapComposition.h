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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Depth = 300.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	bool Flip_X = false;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	bool Flip_Y = true;
	// Size of the heightmaps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Size_X = 512.000000;
	// Size of the heightmaps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Size_Y = 512.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x0_y0 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x0_y0.heightmap_x0_y0"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x0_y1 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x0_y1.heightmap_x0_y1"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x1_y0 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x1_y0.heightmap_x1_y0"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UVoxelHeightmapAssetUINT16> heightmap_x1_y1 = TSoftObjectPtr<UVoxelHeightmapAssetUINT16>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/HeightmapComposition/heightmap_x1_y1.heightmap_x1_y1"));
	
	UVoxelExample_HeightmapComposition();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
