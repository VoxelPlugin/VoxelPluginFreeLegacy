// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"

namespace FVoxelUtilities
{
	template<typename T>
	FORCEINLINE int64 FloorToInt64(T Value)
	{
		return int64(std::floor(Value));
	}
	template<typename T>
	FORCEINLINE int64 FloorToInt32(T Value)
	{
		const int64 Int = FloorToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}
	
	template<typename T>
	FORCEINLINE int64 CeilToInt64(T Value)
	{
		return int64(std::ceil(Value));
	}
	template<typename T>
	FORCEINLINE int64 CeilToInt32(T Value)
	{
		const int64 Int = CeilToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}
	
	template<typename T>
	FORCEINLINE int64 RoundToInt64(T Value)
	{
#if PLATFORM_ANDROID
		// Android NDK doesn't have std::round :(
		return int64(round(Value));
#else
		return int64(std::round(Value));
#endif
	}
	template<typename T>
	FORCEINLINE int64 RoundToInt32(T Value)
	{
		const int64 Int = RoundToInt64(Value);
		ensure(MIN_int32 <= Int && Int <= MAX_int32);
		return int32(Int);
	}

	template<typename TVector>
	FORCEINLINE FIntVector RoundToInt(const TVector& Vector)
	{
		return FIntVector(
			RoundToInt32(Vector.X),
			RoundToInt32(Vector.Y),
			RoundToInt32(Vector.Z));
	}
	template<typename TVector>
	FORCEINLINE FIntVector FloorToInt(const TVector& Vector)
	{
		return FIntVector(
			FloorToInt32(Vector.X),
			FloorToInt32(Vector.Y),
			FloorToInt32(Vector.Z));
	}
	template<typename TVector>
	FORCEINLINE FIntVector CeilToInt(const TVector& Vector)
	{
		return FIntVector(
			CeilToInt32(Vector.X),
			CeilToInt32(Vector.Y),
			CeilToInt32(Vector.Z));
	}

	template<typename TVector>
	FORCEINLINE TVector Abs(const TVector& Vector)
	{
		return TVector(
			FMath::Abs(Vector.X),
			FMath::Abs(Vector.Y),
			FMath::Abs(Vector.Z));
	}

	template<typename TVector>
	FORCEINLINE TVector ComponentMax(const TVector& A, const TVector& B)
	{
		return TVector(
			FMath::Max(A.X, B.X),
			FMath::Max(A.Y, B.Y),
			FMath::Max(A.Z, B.Z));
	}
	template<typename TVector>
	FORCEINLINE TVector ComponentMin(const TVector& A, const TVector& B)
	{
		return TVector(
			FMath::Min(A.X, B.X),
			FMath::Min(A.Y, B.Y),
			FMath::Min(A.Z, B.Z));
	}

	template<typename TVector>
	FORCEINLINE TVector ComponentMin3(const TVector& A, const TVector& B, const TVector& C)
	{
		return ComponentMin(A, ComponentMin(B, C));
	}
	template<typename TVector>
	FORCEINLINE TVector ComponentMax3(const TVector& A, const TVector& B, const TVector& C)
	{
		return ComponentMax(A, ComponentMax(B, C));
	}

	template<typename TVector>
	FORCEINLINE TVoxelStaticArray<FIntVector, 8> GetNeighbors(const TVector& P)
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