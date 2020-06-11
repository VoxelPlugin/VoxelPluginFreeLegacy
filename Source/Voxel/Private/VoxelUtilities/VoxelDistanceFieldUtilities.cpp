// Copyright 2020 Phyronnaz

#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.inl"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"
#include "VoxelShaders/VoxelDistanceFieldShader.h"

void FVoxelDistanceFieldUtilities::ComputeDistanceField_GPU(
	const FIntVector& Size,
	TArray<FFloat16>& InOutData,
	EVoxelDistanceFieldInputType InputType,
	int32 NumberOfPasses)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	check(InOutData.Num() == Size.X * Size.Y * Size.Z);

	const auto DataPtr = MakeVoxelShared<TArray<FFloat16>>(MoveTemp(InOutData));
	const auto Helper = MakeVoxelShared<FVoxelDistanceFieldShaderHelper>();

	Helper->StartCompute(Size, DataPtr, NumberOfPasses, InputType == EVoxelDistanceFieldInputType::Densities);
	Helper->WaitForCompletion();

	InOutData = MoveTemp(*DataPtr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldUtilities::ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const float> Densities, TArrayView<float> OutDistances)
{
	ConvertDensitiesToDistances(Size, Densities, OutDistances, [](float F) { return F; });
}

void FVoxelDistanceFieldUtilities::ConvertDensitiesToDistances(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArrayView<float> OutDistances)
{
	ConvertDensitiesToDistances(Size, Densities, OutDistances, [](FVoxelValue F) { return F.ToFloat(); });
}

void FVoxelDistanceFieldUtilities::ExpandDistanceField(const FIntVector& Size, TArrayView<float> InOutData)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	check(InOutData.Num() == Size.X * Size.Y * Size.Z);
	
#if VOXEL_DEBUG
	auto& Data = InOutData;
#else
	auto* RESTRICT Data = InOutData.GetData();
#endif
	
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				const FIntVector Position(X, Y, Z);
	
				float& Distance = Data[Position.X + Position.Y * Size.X + Position.Z * Size.X * Size.Y];

#define	CheckNeighbor(DX, DY, DZ) \
				{ \
					const FIntVector NeighborPosition = Position + FIntVector(DX, DY, DZ); \
					\
					if (0 <= NeighborPosition.X && NeighborPosition.X < Size.X && \
						0 <= NeighborPosition.Y && NeighborPosition.Y < Size.Y && \
						0 <= NeighborPosition.Z && NeighborPosition.Z < Size.Z) \
					{ \
						const int32 NeighborIndex = NeighborPosition.X + Size.X * NeighborPosition.Y + Size.X * Size.Y * NeighborPosition.Z; \
						const float NeighborDistance = Data[NeighborIndex]; \
						Distance = FMath::Min(Distance, NeighborDistance + 1.f); \
					} \
				}

				CheckNeighbor(-1, 0, 0);
				CheckNeighbor(+1, 0, 0);
				CheckNeighbor(0, -1, 0);
				CheckNeighbor(0, +1, 0);
				CheckNeighbor(0, 0, -1);
				CheckNeighbor(0, 0, +1);

#undef CheckNeighbor
			}
		}
	}
}
