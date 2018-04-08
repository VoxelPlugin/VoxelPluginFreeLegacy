// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FlatWorldGenerator.generated.h"

USTRUCT(Blueprintable)
struct VOXEL_API FFlatWorldLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint8 Material;

	// Start (Z position) of the layer
	UPROPERTY(EditAnywhere)
	int Start;
};

/**
 * Flat world with layers
 */
UCLASS(Blueprintable)
class VOXEL_API UFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFlatWorldGenerator();

	// Altitude of the terrain
	UPROPERTY(EditAnywhere)
	int TerrainHeight;

	// Fade height between different layers
	UPROPERTY(EditAnywhere)
	float FadeHeight;

	// The terrain layers
	UPROPERTY(EditAnywhere)
	TArray<FFlatWorldLayer> TerrainLayers;

	//~ Begin UVoxelWorldGenerator Interface
	TSharedRef<FVoxelWorldGeneratorInstance> GetWorldGenerator() override;
	//~ End UVoxelWorldGenerator Interface
};

class VOXEL_API FFlatWorldGeneratorInstance : public FVoxelWorldGeneratorInstance
{
public:
	FFlatWorldGeneratorInstance(int TerrainHeight, float FadeHeight, const TArray<FFlatWorldLayer>& TerrainLayers);

	//~ Begin FVoxelWorldGeneratorInstance Interface
	void GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, int Step, const FIntVector& Size, const FIntVector& ArraySize) const override;
	bool IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const override;
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const int TerrainHeight;
	const float FadeHeight;
	const TArray<FFlatWorldLayer> TerrainLayers;
};