// Copyright 2020 Phyronnaz

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
			return 1;
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
	inline int32 GetDepthFromBounds(const FIntBox& Bounds)
	{
		return GetDepthFromSize<ChunkSize>(Bounds.Size().GetMax());
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromDepth(int32 Depth)
	{
		const FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FIntBox(-Size, Size);
	}
	
	template<uint32 ChunkSize>
	inline FIntBox GetCustomBoundsForDepth(FIntBox Bounds, int32 Depth)
	{
		Bounds = Bounds.MakeMultipleOfBigger(ChunkSize);
		Bounds = FVoxelUtilities::GetBoundsFromDepth<ChunkSize>(Depth).Overlap(Bounds);
		check(Bounds.IsMultipleOf(ChunkSize));
		return Bounds;
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromPositionAndDepth(const FIntVector& Position, int32 Depth)
	{
		return FIntBox(Position, Position + FIntVector(ChunkSize << Depth));
	}

	// Valid for root node only
	template<uint32 ChunkSize>
	inline int32 GetOctreeDepthContainingBounds(const FIntBox& Bounds)
	{
		const int32 Max = FMath::Max(FVoxelUtilities::Abs(Bounds.Min).GetMax(), FVoxelUtilities::Abs(Bounds.Max).GetMax());
		return GetDepthFromSize<ChunkSize>(2 * Max); // 2x: octree doesn't start at 0 0 0
	}

	constexpr int32 DataOctreeDepthDiff = DATA_CHUNK_SIZE <= RENDER_CHUNK_SIZE ? IntLog2(RENDER_CHUNK_SIZE / DATA_CHUNK_SIZE) : -IntLog2(DATA_CHUNK_SIZE / RENDER_CHUNK_SIZE);

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

	template<typename T>
	inline T MergeAsset(T A, T B, bool bSubtractiveAsset)
	{
		return bSubtractiveAsset ? FMath::Max(A, B) : FMath::Min(A, B);
	}

	inline float SmoothUnion(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f + 0.5f * (DistanceB - DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, DistanceA, H) - K * H * (1.0f - H);
	}
	inline float SmoothSubtraction(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f - 0.5f * (DistanceB + DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, -DistanceA, H) + K * H * (1.0f - H);
	}
	inline float SmoothIntersection(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f - 0.5f * (DistanceB - DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, DistanceA, H) + K * H * (1.0f - H);
	}
}