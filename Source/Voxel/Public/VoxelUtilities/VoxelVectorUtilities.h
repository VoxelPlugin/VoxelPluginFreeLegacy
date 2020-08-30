// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelVector.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

namespace FVoxelUtilities
{
	FORCEINLINE int64 FloorToInt64(v_flt Value)
	{
		return int64(std::floor(Value));
	}
	FORCEINLINE int64 FloorToInt32(v_flt Value)
	{
		const int64 Int = FloorToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}
	
	FORCEINLINE int64 CeilToInt64(v_flt Value)
	{
		return int64(std::ceil(Value));
	}
	FORCEINLINE int64 CeilToInt32(v_flt Value)
	{
		const int64 Int = CeilToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}
	
	FORCEINLINE int64 RoundToInt64(v_flt Value)
	{
#if PLATFORM_ANDROID
		// Android NDK doesn't have std::round :(
		return int64(round(Value));
#else
		return int64(std::round(Value));
#endif
	}
	FORCEINLINE int64 RoundToInt32(v_flt Value)
	{
		const int64 Int = RoundToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}
	
	FORCEINLINE FIntVector RoundToInt(const FVoxelVector& Vector)
	{
		return FIntVector(
			RoundToInt32(Vector.X),
			RoundToInt32(Vector.Y),
			RoundToInt32(Vector.Z));
	}
	FORCEINLINE FIntVector FloorToInt(const FVoxelVector& Vector)
	{
		return FIntVector(
			FloorToInt32(Vector.X),
			FloorToInt32(Vector.Y),
			FloorToInt32(Vector.Z));
	}
	FORCEINLINE FIntVector CeilToInt(const FVoxelVector& Vector)
	{
		return FIntVector(
			CeilToInt32(Vector.X),
			CeilToInt32(Vector.Y),
			CeilToInt32(Vector.Z));
	}

	FORCEINLINE FVoxelVector Abs(const FVoxelVector& Vector)
	{
		return FVoxelVector(
			FMath::Abs(Vector.X),
			FMath::Abs(Vector.Y),
			FMath::Abs(Vector.Z));
	}

	FORCEINLINE FVoxelVector ComponentMax(const FVoxelVector& A, const FVoxelVector& B)
	{
		return FVoxelVector(
			FMath::Max(A.X, B.X),
			FMath::Max(A.Y, B.Y),
			FMath::Max(A.Z, B.Z));
	}
	FORCEINLINE FVoxelVector ComponentMin(const FVoxelVector& A, const FVoxelVector& B)
	{
		return FVoxelVector(
			FMath::Min(A.X, B.X),
			FMath::Min(A.Y, B.Y),
			FMath::Min(A.Z, B.Z));
	}

	FORCEINLINE FVoxelVector ComponentMin3(const FVoxelVector& A, const FVoxelVector& B, const FVoxelVector& C)
	{
		return ComponentMin(A, ComponentMin(B, C));
	}
	FORCEINLINE FVoxelVector ComponentMax3(const FVoxelVector& A, const FVoxelVector& B, const FVoxelVector& C)
	{
		return ComponentMax(A, ComponentMax(B, C));
	}

	FORCEINLINE TVoxelStaticArray<FIntVector, 8> GetNeighbors(const FVoxelVector& P)
	{
		const int32 MinX = FloorToInt32(P.X);
		const int32 MinY = FloorToInt32(P.Y);
		const int32 MinZ = FloorToInt32(P.Z);

		const int32 MaxX = CeilToInt32(P.X);
		const int32 MaxY = CeilToInt32(P.Y);
		const int32 MaxZ = CeilToInt32(P.Z);

		return
		{
			FIntVector(MinX, MinY, MinZ),
			FIntVector(MaxX, MinY, MinZ),
			FIntVector(MinX, MaxY, MinZ),
			FIntVector(MaxX, MaxY, MinZ),
			FIntVector(MinX, MinY, MaxZ),
			FIntVector(MaxX, MinY, MaxZ),
			FIntVector(MinX, MaxY, MaxZ),
			FIntVector(MaxX, MaxY, MaxZ)
		};
	}
}