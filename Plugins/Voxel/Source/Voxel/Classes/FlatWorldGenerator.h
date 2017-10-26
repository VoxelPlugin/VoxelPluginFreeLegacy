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
		uint8 Material1;

	UPROPERTY(EditAnywhere)
		uint8 Material2;

	UPROPERTY(EditAnywhere)
		float Alpha;

	// Start (Z position) of the layer
	UPROPERTY(EditAnywhere)
		int Start;

	// Height of the layer
	UPROPERTY(EditAnywhere)
		int Height;
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
		int Height;

	UPROPERTY(EditAnywhere)
		uint8 DefaultMaterial1;
	UPROPERTY(EditAnywhere)
		uint8 DefaultMaterial2;
	UPROPERTY(EditAnywhere)
		float DefaultAlpha;

	// Layers
	UPROPERTY(EditAnywhere)
		TArray<FFlatWorldStruct> TerrainLayers;

	// Value to set
	UPROPERTY(EditAnywhere)
		float ValueMultiplier;
};