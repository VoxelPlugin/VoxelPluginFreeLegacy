// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

template<class T>
struct TVoxelArray3
{
	FIntVector Size = FIntVector(ForceInit);
	TArray<T> Data;

	TVoxelArray3() = default;
	explicit TVoxelArray3(const FIntVector& NewSize)
	{
		Resize(NewSize);
	}
	
	FORCEINLINE const T& operator()(const FIntVector& P) const
	{
		return (*this)(P.X, P.Y, P.Z);
	}
	FORCEINLINE T& operator()(const FIntVector& P)
	{
		return (*this)(P.X, P.Y, P.Z);
	}
	
	FORCEINLINE const T& operator()(int32 I, int32 J, int32 K) const
	{
		return const_cast<TVoxelArray3<T>&>(*this)(I, J, K);
	}
	FORCEINLINE T& operator()(int32 I, int32 J, int32 K)
	{
		checkVoxelSlow(0 <= I && I < Size.X);
		checkVoxelSlow(0 <= J && J < Size.Y);
		checkVoxelSlow(0 <= K && K < Size.Z);
		return Data.GetData()[I + J * Size.X + K * Size.X * Size.Y];
	}

	void Resize(const FIntVector& NewSize)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		check(int64(NewSize.X) * int64(NewSize.Y) * int64(NewSize.Z) < MAX_int32);
		Size = NewSize;
		Data.Empty(Size.X * Size.Y * Size.Z);
		Data.SetNumUninitialized(Size.X * Size.Y * Size.Z);
	}

	void Assign(const T& Value)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		for (auto& X : Data)
		{
			X = Value;
		}
	}

	void Memzero()
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		FMemory::Memzero(Data.GetData(), Data.Num() * sizeof(T));
	}
};