// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "LandscapeWorldGenerator.generated.h"

class ALandscapeVoxelAsset;

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API ULandscapeWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		ALandscapeVoxelAsset* Landscape;

	virtual float GetDefaultValue(int X, int Y, int Z) override;

	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;

	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;
};
