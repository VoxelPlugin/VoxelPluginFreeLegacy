// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelIntVectorUtilities.h"

namespace FVoxelUtilities
{
	template<uint32 ChunkSize>
	inline int32 GetDepthFromSize(int32 Size)
	{
		if (Size <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::Clamp(FMath::CeilToInt(FMath::Log2(Size / float(ChunkSize))), 1, MAX_WORLD_DEPTH - 1);
		}
	}
	
	template<uint32 ChunkSize>
	inline constexpr int32 GetSizeFromDepth(int32 Depth)
	{
		return ChunkSize << Depth;
	}

	template<uint32 ChunkSize>
	inline constexpr FIntBox GetBoundsFromDepth(int32 Depth)
	{
		FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FIntBox(-Size, Size);
	}

	template<uint32 ChunkSize>
	inline constexpr FIntBox GetBoundsFromPositionAndDepth(const FIntVector& Position, int32 Depth)
	{
		return FIntBox(Position, Position + FIntVector(ChunkSize << Depth));
	}

	// Valid for root node only
	template<uint32 ChunkSize>
	inline int32 GetOctreeDepthContainingBounds(const FIntBox& Bounds)
	{
		int32 Max = FMath::Max(FVoxelUtilities::Abs(Bounds.Min).GetMax(), FVoxelUtilities::Abs(Bounds.Max).GetMax());
		return GetDepthFromSize<ChunkSize>(2 * Max); // 2x: octree doesn't start at 0 0 0
	}

	inline constexpr int32 IntLog2(int32 X)
	{
		int32 Exp = -1;
		while (X)
		{
			X >>= 1;
			++Exp;
		}
		return Exp;
	}

	constexpr int32 DataOctreeDepthDiff = VOXEL_CELL_SIZE <= CHUNK_SIZE ? IntLog2(CHUNK_SIZE / VOXEL_CELL_SIZE) : -IntLog2(VOXEL_CELL_SIZE / CHUNK_SIZE);

	inline constexpr int32 GetDataDepthFromChunkDepth(int32 Depth)
	{
		return Depth + DataOctreeDepthDiff;
	}

	inline constexpr int32 GetChunkDepthFromDataDepth(int32 Depth)
	{
		return Depth - DataOctreeDepthDiff;
	}

	inline int32 ClampChunkDepth(int32 Depth)
	{
		return FMath::Clamp(Depth, 0, MAX_WORLD_DEPTH - 1);
	}

	inline int32 ClampDataDepth(int32 Depth)
	{
		return FMath::Clamp(Depth, 1, GetDataDepthFromChunkDepth(MAX_WORLD_DEPTH) - 1);
	}

	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 *  -----> X
	 */
	template<typename T, typename U = float>
	inline T BilinearInterpolation(T A, T B, T C, T D, U X, U Y)
	{
		T AB = FMath::Lerp<T, U>(A, B, X);
		T CD = FMath::Lerp<T, U>(C, D, X);
		return FMath::Lerp<T, U>(AB, CD, Y);
	}
	
	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 * 0-----> X
	 * Y
	 * ^ G - H
	 * | |   |
	 * | E - F
	 * 1-----> X
	 */
	template<typename T, typename U = float>
	inline T TrilinearInterpolation(
		T A, T B, T C, T D,
		T E, T F, T G, T H,
		U X, U Y, U Z)
	{
		T ABCD = BilinearInterpolation<T, U>(A, B, C, D, X, Y);
		T EFGH = BilinearInterpolation<T, U>(E, F, G, H, X, Y);
		return FMath::Lerp<T, U>(ABCD, EFGH, Z);
	}

	template<typename T>
	inline void ShuffleArray(T& Array, int32 Seed = 0)
	{
		FRandomStream Stream(Seed);
		for (int32 Index = 0; Index < 256; Index++)
		{
			int32 NewIndex = Stream.RandRange(Index, 255);
			if (Index != NewIndex)
			{
				Array.Swap(Index, NewIndex);
			}
		}
	}
	
	inline uint8 LerpUINT8(uint8 A, uint8 B, float Amount)
	{
		float LerpResult = FMath::Lerp<float>(A, B, Amount);
		// Do special rounding to not get stuck, eg Lerp(251, 255, 0.1) = 251
		int32 RoundedResult = Amount > 0 ? FMath::CeilToInt(LerpResult) : FMath::FloorToInt(LerpResult);
		return FMath::Clamp(RoundedResult, 0, 255);
	}

	inline uint8 FloatToUINT8(float Float)
	{
		return FMath::Clamp(FMath::RoundToInt(Float * 255.999f), 0, 255);
	}

	inline float UINT8ToFloat(uint8 Int)
	{
		return Int / 255.999f;
	}
}