// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelDistanceFieldUtilities.h"

template<typename T, typename TLambda>
void FVoxelDistanceFieldUtilities::ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const T> Densities, TArrayView<float> OutDistances, TLambda GetFloatFromT)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(Densities.Num() == Size.X * Size.Y * Size.Z);
	check(OutDistances.Num() == Size.X * Size.Y * Size.Z);

#if VOXEL_DEBUG
	auto& InData = Densities;
	auto& OutData = OutDistances;
#else
	auto* RESTRICT InData = Densities.GetData();
	auto* RESTRICT OutData = OutDistances.GetData();
#endif

	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				const FIntVector Position(X, Y, Z);

				const int32 Index = Position.X + Size.X * Position.Y + Size.X * Size.Y * Position.Z;

				const float Value = GetFloatFromT(InData[Index]);
				float Distance = 1e20;

#define	CheckNeighbor(DX, DY, DZ) \
				{ \
					const FIntVector NeighborPosition = Position + FIntVector(DX, DY, DZ); \
					\
					if (0 <= NeighborPosition.X && NeighborPosition.X < Size.X && \
						0 <= NeighborPosition.Y && NeighborPosition.Y < Size.Y && \
						0 <= NeighborPosition.Z && NeighborPosition.Z < Size.Z) \
					{ \
						const int32 NeighborIndex = NeighborPosition.X + Size.X * NeighborPosition.Y + Size.X * Size.Y * NeighborPosition.Z; \
						const float NeighborValue = GetFloatFromT(InData[NeighborIndex]); \
						\
						if (Value > 0 != NeighborValue > 0) \
						{ \
							const float SurfaceDistance = Value / (Value - NeighborValue); \
							Distance = FMath::Min(Distance, SurfaceDistance); \
						} \
					} \
				}

				CheckNeighbor(-1, 0, 0);
				CheckNeighbor(+1, 0, 0);
				CheckNeighbor(0, -1, 0);
				CheckNeighbor(0, +1, 0);
				CheckNeighbor(0, 0, -1);
				CheckNeighbor(0, 0, +1);

#undef CheckNeighbor

				OutData[Index] = Distance;
			}
		}
	}
}