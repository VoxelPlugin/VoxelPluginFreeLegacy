// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"

struct FVoxelChunkSettings
{
	bool bVisible : 1;
	bool bEnableCollisions : 1;
	bool bEnableNavmesh : 1;
	uint8 TransitionsMask;

	inline bool HasRenderChunk() const { return bVisible || bEnableCollisions || bEnableNavmesh; }

	inline bool operator!=(const FVoxelChunkSettings& Other) const
	{
		return
			bVisible            != Other.bVisible            ||
			bEnableCollisions   != Other.bEnableCollisions   ||
			bEnableNavmesh      != Other.bEnableNavmesh      ||
			TransitionsMask     != Other.TransitionsMask;
	}
	inline bool operator==(const FVoxelChunkSettings& Other) const
	{
		return
			bVisible            == Other.bVisible            &&
			bEnableCollisions   == Other.bEnableCollisions   &&
			bEnableNavmesh      == Other.bEnableNavmesh      &&
			TransitionsMask     == Other.TransitionsMask;
	}

	inline static FVoxelChunkSettings Visible()
	{
		return { true, false, false, 0 };
	}
	inline static FVoxelChunkSettings VisibleWithCollisions()
	{
		return { true, true, false, 0 };
	}

};

struct FVoxelChunkUpdate
{
	uint64 Id = -1;
	int32 LOD = -1;
	FVoxelIntBox Bounds;
	FVoxelChunkSettings OldSettings;
	FVoxelChunkSettings NewSettings;
	TArray<uint64, TInlineAllocator<8>> PreviousChunks;
};