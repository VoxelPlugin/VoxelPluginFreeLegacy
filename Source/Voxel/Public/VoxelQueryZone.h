// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "StackArray.h"

template<typename T>
class TVoxelQueryZone
{
public:
	const uint32 Step;
	const FIntBox Bounds;

	TVoxelQueryZone(const FIntBox& Bounds, TArray<T>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	template<uint32 Size>
	TVoxelQueryZone(const FIntBox& Bounds, TStackArray<T, Size>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	TVoxelQueryZone(const FIntBox& Bounds, T* Data)
		: TVoxelQueryZone(Bounds, Bounds.Size(), 0, Data)
	{
	}
	TVoxelQueryZone(const FIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, TArray<T>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data.GetData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	template<uint32 Size>
	TVoxelQueryZone(const FIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, TStackArray<T, Size>& Data)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data.GetData())
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(Data.Num() == ArraySize.X * ArraySize.Y * ArraySize.Z);
	}
	TVoxelQueryZone(const FIntBox& Bounds, const FIntVector& ArraySize, int32 LOD, T* Data)
		: TVoxelQueryZone(Bounds, Bounds.Min, ArraySize, LOD, Data)
	{
		check(Bounds.IsValid());
		check(Bounds.Size() / Step == ArraySize);
		check(Data);
	}

	FORCEINLINE void Set(int32 X, int32 Y, int32 Z, T Value)
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

		const int32 Index = LocalX + ArraySize.X * LocalY + ArraySize.X * ArraySize.Y * LocalZ;
		Data[Index] = Value;
	}
	
	TVoxelQueryZone<T> ShrinkTo(const FIntBox& InBounds) const
	{
		FIntBox LocalBounds = Bounds.Overlap(InBounds);
		LocalBounds = LocalBounds.MakeMultipleOfRoundUp(Step);
		return TVoxelQueryZone<T>(LocalBounds, Offset, ArraySize, LOD, Data);
	}

private:
	T* RESTRICT Data;
	const FIntVector Offset;
	const FIntVector ArraySize;
	const uint32 LOD;
	
	TVoxelQueryZone(const FIntBox& Bounds, const FIntVector& Offset, const FIntVector& ArraySize, int32 LOD, T* Data)
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
};

#define VOXEL_QUERY_ZONE_ITERATE(QueryZone, X) int32 X = QueryZone.Bounds.Min.X; X < QueryZone.Bounds.Max.X; X += QueryZone.Step