// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.generated.h"

class AVoxelWorld;

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API AVoxelWorldGenerator : public AActor
{
	GENERATED_BODY()

public:
	/**
	 * Get default value at position
	 * @param	Position		Position in voxel space
	 * @return	Value at position: positive if empty, negative if filled
	 */
	virtual float GetDefaultValue(int X, int Y, int Z);

	/**
	 * Get default color at position
	 * @param	Position		Position in voxel space
	 * @return	Color at position
	 */
	virtual FColor GetDefaultColor(int X, int Y, int Z);

	/**
	 * If need reference to Voxel World
	 */
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld);
};
