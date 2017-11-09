// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FlatWorldGenerator.generated.h"


USTRUCT(Blueprintable)
struct VOXEL_API FFlatWorldStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		uint8 Material;

	// Start (Z position) of the layer
	UPROPERTY(EditAnywhere)
		int Start;
};

FORCEINLINE bool operator<(const FFlatWorldStruct& Left, const FFlatWorldStruct& Right)
{
	return Left.Material < Right.Material;
}

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
		int Height;

	UPROPERTY(EditAnywhere)
		uint8 DefaultMaterial;

	UPROPERTY(EditAnywhere)
		float FadeHeight;

	// Layers
	UPROPERTY(EditAnywhere)
		TArray<FFlatWorldStruct> TerrainLayers;

	// Value to set
	UPROPERTY(EditAnywhere)
		float HardnessMultiplier;
};