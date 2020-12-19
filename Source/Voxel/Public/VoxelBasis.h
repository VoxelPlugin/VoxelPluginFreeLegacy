// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVector.h"

struct FVoxelIntBox;

struct VOXEL_API FVoxelBasis
{
	FVoxelVector X = FVoxelVector(ForceInit);
	FVoxelVector Y = FVoxelVector(ForceInit);
	FVoxelVector Z = FVoxelVector(ForceInit);

	FVoxelBasis() = default;
	FVoxelBasis(const FVoxelVector& X, const FVoxelVector& Y, const FVoxelVector& Z)
		: X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	// Returns a basis that can be used to iterate the bounds (X/Y will roughly follow the planet ground in the bounds)
	static FVoxelBasis MakePlanetBasisForBounds(const FVoxelIntBox& Bounds);
	static FVoxelBasis Unit();
};