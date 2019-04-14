// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

struct VOXELGRAPH_API FVoxelContext
{
	const FVoxelPlaceableItemHolder& ItemHolder;
	const int32 LOD;
	int32 X;
	int32 Y;
	int32 Z;

	FVoxelContext(const FVoxelPlaceableItemHolder& ItemHolder, int32 LOD, int32 X = 0, int32 Y = 0, int32 Z = 0)
		: ItemHolder(ItemHolder)
		, LOD(LOD)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	FVoxelContext(int32 X, int32 Y, int32 Z)
		: ItemHolder(StaticItem)
		, LOD(0)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	FVoxelContext()
		: ItemHolder(StaticItem)
		, LOD(0)
		, X(0)
		, Y(0)
		, Z(0)
	{
	}

private:
	static const FVoxelPlaceableItemHolder StaticItem;
};

struct VOXELGRAPH_API FVoxelContextRange
{
	const int32 LOD;
	TVoxelRange<int32> X;
	TVoxelRange<int32> Y;
	TVoxelRange<int32> Z;
	const FVoxelPlaceableItemHolder ItemHolder;

	FVoxelContextRange()
		: LOD(0)
		, X(0)
		, Y(0)
		, Z(0)
	{
	}
	FVoxelContextRange(int32 LOD, TVoxelRange<int32> X, TVoxelRange<int32> Y, TVoxelRange<int32> Z )
		: LOD(LOD)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}
};