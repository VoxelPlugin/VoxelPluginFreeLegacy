// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"

class VOXEL_API FVoxelLevelToolsImpl : public FVoxelToolsBaseImpl
{
public:
	static FVoxelIntBox GetBounds(const FVoxelVector& Position, float Radius, float Height, bool bAdditive);
	
public:
	/**
	 * Stamps a cylinder, to quickly level parts of the world
	 * If additive, will stamp a smooth cylinder above Position. Else will remove one below Position
	 * @param	Position	The position of the top (or bottom if subtractive) of the cylinder @VoxelPosition @GetBounds
	 * @param	Radius		The radius of the cylinder @VoxelDistance @GetBounds
	 * @param	Falloff		The falloff between 0 and 1. The higher the smoother the cylinder edge.
	 * @param	Height		The height of the cylinder @VoxelDistance @GetBounds
	 * @param	bAdditive	Additive or subtractive edit, see node comment @GetBounds
	 * @ExportSetValue
	 */
	template<typename TData>
	static void Level(
		TData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Falloff,
		float Height,
		bool bAdditive);
};