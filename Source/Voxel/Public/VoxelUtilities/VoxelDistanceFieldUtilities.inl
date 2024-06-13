// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"

template<typename T, typename TLambda>
void FVoxelDistanceFieldUtilities::GetSurfacePositionsFromDensities(
	const FIntVector& Size,
	TArrayView<const T> Densities,
	TArrayView<float> OutDistances,
	TArrayView<FVector3f> OutSurfacePositions,
	TLambda GetFloatFromT)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const FIntVector DensitiesSize = Size + 2;
	const FIntVector DensitiesOffset = FIntVector(-1, -1, -1);
	
	check(Densities.Num() == DensitiesSize.X * DensitiesSize.Y * DensitiesSize.Z);
	check(OutDistances.Num() == Size.X * Size.Y * Size.Z);
	check(OutSurfacePositions.Num() == Size.X * Size.Y * Size.Z);

	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				const FIntVector Position(X, Y, Z);

				const float Value = GetFloatFromT(FVoxelUtilities::Get3D(Densities, DensitiesSize, Position, DensitiesOffset));
				
				const int32 Index = FVoxelUtilities::Get3DIndex(Size, Position);
				FVoxelUtilities::Get(OutDistances, Index) = FMath::Sign(Value);

				// Static branch
				const auto Lambda = [&](auto IsNegative)
				{
					// Take the max: this is the one that will "push" the value the closest to us
					// Only consider positive values, so that there's a surface between us
					// By symmetry, take the min value negative if Value is positive
					float MaxNeighborValue = 0.f;
					FIntVector MaxNeighborPosition = FIntVector(ForceInit);

#define	CheckNeighbor(DX, DY, DZ) \
					{ \
						const FIntVector NeighborPosition = Position + FIntVector(DX, DY, DZ); \
						const float NeighborValue = GetFloatFromT(FVoxelUtilities::Get3D(Densities, DensitiesSize, NeighborPosition, DensitiesOffset)); \
						\
						if (IsNegative ? (NeighborValue > MaxNeighborValue) : (NeighborValue < MaxNeighborValue)) \
						{ \
							MaxNeighborValue = NeighborValue; \
							MaxNeighborPosition = NeighborPosition; \
						} \
					}

					CheckNeighbor(-1, 0, 0);
					CheckNeighbor(+1, 0, 0);
					CheckNeighbor(0, -1, 0);
					CheckNeighbor(0, +1, 0);
					CheckNeighbor(0, 0, -1);
					CheckNeighbor(0, 0, +1);

#undef CheckNeighbor

					if (MaxNeighborValue == 0.f)
					{
						FVoxelUtilities::Get(OutSurfacePositions, Index) = MakeInvalidSurfacePosition();
					}
					else
					{
						const float Alpha = Value / (Value - MaxNeighborValue);
						FVoxelUtilities::Get(OutSurfacePositions, Index) = FMath::Lerp(FVector3f(Position), FVector3f(MaxNeighborPosition), Alpha);
					}
				};
				
				if (Value <= 0)
				{
					Lambda(FVoxelUtilities::FTrueType());
				}
				else
				{
					Lambda(FVoxelUtilities::FFalseType());
				}
			}
		}
	}
}