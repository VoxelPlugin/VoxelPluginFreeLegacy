// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FlatWorldGenerator.generated.h"


USTRUCT(Blueprintable)
struct VOXEL_API FFlatWorldLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint8 Material;

	// Start (Z position) of the layer
	UPROPERTY(EditAnywhere)
		int Start;
};

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFlatWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	// Height of the difference between full and empty
	UPROPERTY(EditAnywhere)
		int TerrainHeight;

	UPROPERTY(EditAnywhere)
		float FadeHeight;

	// Layers
	UPROPERTY(EditAnywhere)
		TArray<FFlatWorldLayer> TerrainLayers;

	// Value to set
	UPROPERTY(EditAnywhere)
		float HardnessMultiplier;
};