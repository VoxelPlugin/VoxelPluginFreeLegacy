// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "EmptyWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UEmptyWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	virtual float GetDefaultValue(int X, int Y, int Z) override;

	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;

	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;
};
