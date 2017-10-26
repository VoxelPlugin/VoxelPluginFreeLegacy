// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelWorldGenerator.generated.h"

class AVoxelWorld;

/**
 *
 */
UCLASS(Blueprintable, abstract, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering"))
class VOXEL_API UVoxelWorldGenerator : public UObject
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
	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z);

	/**
	 * If you need a reference to Voxel World
	 */
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld);

	/**
	 * World up vector at position
	 */
	virtual FVector GetUpVector(int X, int Y, int Z);
};
