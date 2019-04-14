// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"

struct FVoxelWorldGeneratorQueryZone
{
	const FIntBox Bounds;
	const FIntVector Offset;
	const FIntVector ArraySize;
	const int32 LOD;
	const int32 Step;

	FVoxelWorldGeneratorQueryZone(const FIntBox& Bounds, const FIntVector& Offset, const FIntVector& ArraySize, int32 LOD)
		: Bounds(Bounds)
		, Offset(Offset)
		, ArraySize(ArraySize)
		, LOD(LOD)
		, Step(1 << LOD)
	{
		check(Bounds.IsMultipleOf(Step));
	}
	FVoxelWorldGeneratorQueryZone(const FIntVector& Position)
		: FVoxelWorldGeneratorQueryZone(FIntBox(Position), Position, FIntVector(1, 1, 1), 0)
	{
	}
	FVoxelWorldGeneratorQueryZone(int32 X, int32 Y, int32 Z)
		: FVoxelWorldGeneratorQueryZone(FIntVector(X, Y, Z))
	{
	}
	FVoxelWorldGeneratorQueryZone(const FIntBox& Bounds, const FIntVector& ArraySize, int32 LOD)
		: FVoxelWorldGeneratorQueryZone(Bounds, Bounds.Min, ArraySize, LOD)
	{
		check(Bounds.Size() / Step == ArraySize);
	}

	inline int32 GetIndex(int32 X, int32 Y, int32 Z) const
	{
		int32 LX = (uint32)(X - Offset.X) >> LOD;
		int32 LY = (uint32)(Y - Offset.Y) >> LOD;
		int32 LZ = (uint32)(Z - Offset.Z) >> LOD;

		checkVoxelSlow(LX == (X - Offset.X) / Step);
		checkVoxelSlow(LY == (Y - Offset.Y) / Step);
		checkVoxelSlow(LZ == (Z - Offset.Z) / Step);

		checkVoxelSlow(0 <= LX && LX < ArraySize.X);
		checkVoxelSlow(0 <= LY && LY < ArraySize.Y);
		checkVoxelSlow(0 <= LZ && LZ < ArraySize.Z);

		return LX + ArraySize.X * LY + ArraySize.X * ArraySize.Y * LZ;
	}

	inline int32 GetCache2DIndex(int32 X, int32 Y) const
	{
		checkVoxelSlow(LOD == 0);
		int32 Index = (X - Bounds.Min.X) + (Bounds.Max.X - Bounds.Min.X) * (Y - Bounds.Min.Y);
		checkVoxelSlow(0 <= Index && Index < VOXEL_CELL_SIZE * VOXEL_CELL_SIZE);
		return Index;
	}

	inline FVoxelWorldGeneratorQueryZone ShrinkTo(const FIntBox& InBounds) const
	{
		FIntBox LocalBounds = Bounds.Overlap(InBounds);
		LocalBounds.MakeMultipleOfExclusive(Step);
		return FVoxelWorldGeneratorQueryZone(LocalBounds, Offset, ArraySize, LOD);
	}

public:
	struct FIteratorElement
	{
		int32 Value;
		const int32 Step;

		FIteratorElement(int32 Value, int32 Step = -1) : Value(Value), Step(Step) {}

		inline int32 operator*() const
		{
			return Value;
		}
		inline void operator++()
		{
			Value += Step;
		}
		inline bool operator!=(const FIteratorElement& Other) const
		{
			checkVoxelSlow(Other.Step == -1);
			return Value < Other.Value;
		}
	};

	struct FIterator
	{
		const int32 Min;
		const int32 Max;
		const int32 Step;

		FIterator(int32 Min, int32 Max, int32 Step) : Min(Min), Max(Max), Step(Step) {}

		FIteratorElement begin() { return FIteratorElement(Min, Step); }
		FIteratorElement end() { return FIteratorElement(Max); }
	};

	inline FIterator XIt() const { return FIterator(Bounds.Min.X, Bounds.Max.X, Step); }
	inline FIterator YIt() const { return FIterator(Bounds.Min.Y, Bounds.Max.Y, Step); }
	inline FIterator ZIt() const { return FIterator(Bounds.Min.Z, Bounds.Max.Z, Step); }
};

struct FVoxelWorldGeneratorQueryZone2D
{
	const FIntPoint Min;
	const FIntPoint Max; // Excluded

	FVoxelWorldGeneratorQueryZone2D(const FIntPoint& Min, const FIntPoint& Max)
		: Min(Min)
		, Max(Max)
	{
		check(Min.X < Max.X);
		check(Min.Y < Max.Y);
		check(Max.X - Min.X == VOXEL_CELL_SIZE);
		check(Max.Y - Min.Y == VOXEL_CELL_SIZE);
	}
	
	inline int32 GetIndex(int32 X, int32 Y) const
	{
		int32 Index = (X - Min.X) + (Max.X - Min.X) * (Y - Min.Y);
		checkVoxelSlow(0 <= Index && Index < VOXEL_CELL_SIZE * VOXEL_CELL_SIZE);
		return Index;
	}

public:
	struct FIteratorElement
	{
		int32 Value;

		FIteratorElement(int32 Value) : Value(Value) {}

		inline int32 operator*() const
		{
			return Value;
		}
		inline void operator++()
		{
			Value++;
		}
		inline bool operator!=(const FIteratorElement& Other) const
		{
			return Value < Other.Value;
		}
	};

	struct FIterator
	{
		const int32 Min;
		const int32 Max;

		FIterator(int32 Min, int32 Max) : Min(Min), Max(Max) {}

		FIteratorElement begin() { return FIteratorElement(Min); }
		FIteratorElement end() { return FIteratorElement(Max); }
	};

	inline FIterator XIt() const { return FIterator(Min.X, Max.X); }
	inline FIterator YIt() const { return FIterator(Min.Y, Max.Y); }
};