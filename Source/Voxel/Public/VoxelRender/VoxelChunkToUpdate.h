// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

struct FVoxelChunkSettings
{
	bool bVisible : 1;
	bool bEnableCollisions : 1;
	bool bEnableNavmesh : 1;
	bool bEnableTessellation : 1;
	uint8 TransitionsMask;

	inline bool HasRenderChunk() const { return bVisible || bEnableCollisions || bEnableNavmesh; }

	inline bool operator!=(const FVoxelChunkSettings& Other) const
	{
		return
			bVisible            != Other.bVisible            ||
			bEnableCollisions   != Other.bEnableCollisions   ||
			bEnableNavmesh      != Other.bEnableNavmesh      ||
			bEnableTessellation != Other.bEnableTessellation ||
			TransitionsMask     != Other.TransitionsMask;
	}
	inline bool operator==(const FVoxelChunkSettings& Other) const
	{
		return
			bVisible            == Other.bVisible            &&
			bEnableCollisions   == Other.bEnableCollisions   &&
			bEnableNavmesh      == Other.bEnableNavmesh      &&
			bEnableTessellation == Other.bEnableTessellation &&
			TransitionsMask     == Other.TransitionsMask;
	}

	inline static FVoxelChunkSettings Visible()
	{
		return { true, false, false, false, 0 };
	}
	inline static FVoxelChunkSettings VisibleWithCollisions(bool bEnableTessellation)
	{
		return { true, true, false, bEnableTessellation, 0 };
	}

};

struct FVoxelChunkUpdate
{
	uint64 Id = -1;
	int32 LOD = -1;
	FIntBox Bounds;
	FVoxelChunkSettings OldSettings;
	FVoxelChunkSettings NewSettings;
	TArray<uint64, TInlineAllocator<8>> PreviousChunks;
};