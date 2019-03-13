// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "IntBox.h"

namespace FVoxelUtilities
{
	template<uint32 ChunkSize>
	inline int GetDepthFromSize(int Size)
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
	inline int GetSizeFromDepth(int Depth)
	{
		return ChunkSize << Depth;
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromDepth(int Depth)
	{
		FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FIntBox(-Size, Size);
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromPositionAndLOD(const FIntVector& Position, int LOD)
	{
		return FIntBox(Position, Position + FIntVector(ChunkSize << LOD));
	}

	inline int ClampChunkLOD(int LOD)
	{
		return FMath::Clamp(LOD, 0, MAX_WORLD_DEPTH - 1);
	}

	inline int ClampDataLOD(int LOD)
	{
		return FMath::Clamp(LOD, 1, MAX_WORLD_DEPTH - DATA_OCTREE_DEPTH_DIFF - 1);
	}

	inline void AddNeighborsToArray(const FIntVector& V, TArray<FIntVector>& Array)
	{
		const int32& X = V.X;
		const int32& Y = V.Y;
		const int32& Z = V.Z;

		uint32 Pos = Array.AddUninitialized(6);
		FIntVector* Ptr = Array.GetData() + Pos;

		new (Ptr++) FIntVector(X - 1, Y, Z);
		new (Ptr++) FIntVector(X + 1, Y, Z);

		new (Ptr++) FIntVector(X, Y - 1, Z);
		new (Ptr++) FIntVector(X, Y + 1, Z);

		new (Ptr++) FIntVector(X, Y, Z - 1);
		new (Ptr++) FIntVector(X, Y, Z + 1);

		check(Ptr == Array.GetData() + Array.Num());
	}

	inline TArray<FIntVector, TFixedAllocator<8>> GetNeighbors(const FVector& P)
	{
		return {
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::FloorToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::FloorToInt(P.Y), FMath::CeilToInt(P.Z)),
		FIntVector(FMath::FloorToInt(P.X), FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z)),
		FIntVector(FMath::CeilToInt(P.X) , FMath::CeilToInt(P.Y) , FMath::CeilToInt(P.Z))
		};
	}
}