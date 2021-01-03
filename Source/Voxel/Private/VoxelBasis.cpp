// Copyright 2021 Phyronnaz

#include "VoxelBasis.h"
#include "VoxelIntBox.h"

FVoxelBasis FVoxelBasis::MakePlanetBasisForBounds(const FVoxelIntBox& Bounds)
{
	// Find closest corner
	const FVoxelVector Direction = -Bounds.GetCenter().GetSafeNormal();

	const FVoxelVector AbsDirection = Direction.GetAbs();
	const float Max = AbsDirection.GetMax();
	const FVoxelVector Vector =
			Max == AbsDirection.X
				? FVoxelVector(0, 1, 0)
				: Max == AbsDirection.Y
				? FVoxelVector(0, 0, 1)
				: FVoxelVector(1, 0, 0);
	FVoxelBasis OutBasis;
	
	OutBasis.X = Direction ^ Vector;
	OutBasis.Y = Direction ^ OutBasis.X;
	OutBasis.Z = Direction;

	OutBasis.X.Normalize();
	OutBasis.Y.Normalize();

	ensure(OutBasis.X.GetAbsMax() > KINDA_SMALL_NUMBER);
	ensure(OutBasis.Y.GetAbsMax() > KINDA_SMALL_NUMBER);

	return OutBasis;
}

FVoxelBasis FVoxelBasis::Unit()
{
	return FVoxelBasis(FVoxelVector(1, 0, 0), FVoxelVector(0, 1, 0), FVoxelVector(0, 0, 1));
}
