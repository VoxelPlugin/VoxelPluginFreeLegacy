// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelContainers/VoxelStaticArray.h"

template<typename T>
class TVoxelQueryZone
{
public:
	const uint32 Step;
	const FVoxelIntBox Bounds;

	template<typename TAllocator>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, TArray<T, TAllocator>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, TVoxelStaticArray<T, Size>* Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, *Data)
	{
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, TVoxelStaticArray<T, Size>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	TVoxelQueryZone(const FVoxelIntBox& Bounds, T* Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	template<typename TAllocator>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, TArray<T, TAllocator>& Data, bool bSkipSizeCheck = false)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data.GetData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(bSkipSizeCheck || Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, TVoxelStaticArray<T, Size>& Data, bool bSkipSizeCheck = false)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data.GetData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(bSkipSizeCheck || Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, T* Data)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data)
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(Data);
	}

	FORCEINLINE void Set(int32 X, int32 Y, int32 Z, T Value)
	{
		Data[GetIndex(X, Y, Z)] = Value;
	}
	
	TVoxelQueryZone<T> ShrinkTo(const FVoxelIntBox& InBounds) const
	{
		FVoxelIntBox LocalBounds = Bounds.Overlap(InBounds);
		LocalBounds = LocalBounds.MakeMultipleOfRoundUp(Step);
		return TVoxelQueryZone<T>(LocalBounds, Offset, ArraySize, LOD, Data);
	}

protected:
	T* RESTRICT Data;
	const FIntVector Offset;
	const FIntVector ArraySize;
	const uint32 LOD;
	
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& Offset, const FIntVector& ArraySize, int32 LOD, T* Data)
		: Step(1 << LOD)
		, Bounds(Bounds)
		, Data(Data)
		, Offset(Offset)
		, ArraySize(ArraySize)
		, LOD(LOD)
	{
		check(Bounds.IsMultipleOf(Step));
		check(FVoxelUtilities::CountIs32Bits(ArraySize));
	}

	FORCEINLINE uint32 GetIndex(int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(Bounds.Contains(X, Y, Z));
		
		checkVoxelSlow(X % Step == 0);
		checkVoxelSlow(Y % Step == 0);
		checkVoxelSlow(Z % Step == 0);
		
		checkVoxelSlow(Offset.X <= X);
		checkVoxelSlow(Offset.Y <= Y);
		checkVoxelSlow(Offset.Z <= Z);

		const int32 LocalX = uint32(X - Offset.X) >> LOD;
		const int32 LocalY = uint32(Y - Offset.Y) >> LOD;
		const int32 LocalZ = uint32(Z - Offset.Z) >> LOD;

		checkVoxelSlow(0 <= LocalX && LocalX < ArraySize.X);
		checkVoxelSlow(0 <= LocalY && LocalY < ArraySize.Y);
		checkVoxelSlow(0 <= LocalZ && LocalZ < ArraySize.Z);

		return LocalX + ArraySize.X * LocalY + ArraySize.X * ArraySize.Y * LocalZ;
	}
};

#if ONE_BIT_VOXEL_VALUE
template<>
class TVoxelQueryZone<FVoxelValue> : private TVoxelQueryZone<uint32>
{
public:
	using Super = TVoxelQueryZone<uint32>;
	using Super::Step;
	using Super::Bounds;
	
	TVoxelQueryZone(const FVoxelIntBox& Bounds, uint32* Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	TVoxelQueryZone(const FVoxelIntBox& Bounds, FVoxelValueArray& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, TVoxelValueStaticArray<Size>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, TVoxelValueStaticArray<Size>* Data)
		: TVoxelQueryZone(Bounds, *Data)
	{
	}
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, FVoxelValueArray& Data, bool bSkipSizeCheck = false)
		: Super(Bounds, Bounds.Min, ArraySize, LOD, Data.GetWordData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(bSkipSizeCheck || Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	template<uint32 Size>
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, TVoxelValueStaticArray<Size>& Data, bool bSkipSizeCheck = false)
		: Super(Bounds, Bounds.Min, ArraySize, LOD, Data.GetWordData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(bSkipSizeCheck || Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	TVoxelQueryZone(const FVoxelIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, uint32* Data)
		: Super(Bounds, Bounds.Min, ArraySize, LOD, Data)
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(Data);
	}

	FORCEINLINE void Set(int32 X, int32 Y, int32 Z, bool bValue)
	{
		const int32 Index = GetIndex(X, Y, Z);

		constexpr uint32 NumBitsPerWord = 32;
		
		const uint32 Mask = 1u << (Index % NumBitsPerWord);
		uint32& Value = Data[Index / NumBitsPerWord];

		if (bValue)
		{
			Value |= Mask;
		}
		else
		{
			Value &= ~Mask;
		}
	}
	
	TVoxelQueryZone<FVoxelValue> ShrinkTo(const FVoxelIntBox& InBounds) const
	{
		return Super::ShrinkTo(InBounds);
	}

private:
	TVoxelQueryZone(TVoxelQueryZone<uint32> Parent)
		: Super(Parent)
	{
	}
};
#endif

#define VOXEL_QUERY_ZONE_ITERATE(QueryZone, X) int32 X = QueryZone.Bounds.Min.X; X < QueryZone.Bounds.Max.X; X += QueryZone.Step