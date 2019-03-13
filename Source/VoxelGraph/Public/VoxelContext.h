// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

struct VOXELGRAPH_API FVoxelContext
{
	const FVoxelPlaceableItemHolder& ItemHolder;
	const int LOD;
	int32 X;
	int32 Y;
	int32 Z;

	FVoxelContext(const FVoxelPlaceableItemHolder& ItemHolder, int LOD, int X = 0, int Y = 0, int Z = 0)
		: ItemHolder(ItemHolder)
		, LOD(LOD)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	FVoxelContext(int X, int Y, int Z)
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
	const int LOD;
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
	FVoxelContextRange(int LOD, TVoxelRange<int32> X, TVoxelRange<int32> Y, TVoxelRange<int32> Z )
		: LOD(LOD)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}
};