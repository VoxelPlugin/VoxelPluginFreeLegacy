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
	// Color of the layer
	UPROPERTY(EditAnywhere)
		FLinearColor Color;

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
class VOXEL_API AFlatWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	AFlatWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FColor GetDefaultColor(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	// Height of the difference between full and empty
	UPROPERTY(EditAnywhere)
		int Height;

	// Color to set if no layer
	UPROPERTY(EditAnywhere)
		FLinearColor DefaultColor;

	// Layers
	UPROPERTY(EditAnywhere)
		TArray<FFlatWorldStruct> TerrainLayers;

	// Value to set
	UPROPERTY(EditAnywhere)
		float ValueMultiplier;
};