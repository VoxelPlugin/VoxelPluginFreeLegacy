// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FVoxelOctreeId
{
	FIntVector Position;
	uint8 Height;

	FORCEINLINE bool operator==(const FVoxelOctreeId& Other) const
	{
		return Position == Other.Position && Height == Other.Height;
	}
	FORCEINLINE bool operator!=(const FVoxelOctreeId& Other) const
	{
		return Position != Other.Position || Height != Other.Height;
	}
};