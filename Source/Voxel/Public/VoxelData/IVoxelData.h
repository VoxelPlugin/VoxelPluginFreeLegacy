// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"

class FVoxelGeneratorInstance;

class IVoxelDataOctreeMemory
{
public:
	struct FDataOctreeMemory
	{
		uint8 PadToAvoidContention0[PLATFORM_CACHE_LINE_SIZE];
		FThreadSafeCounter64 Values;
		uint8 PadToAvoidContention1[PLATFORM_CACHE_LINE_SIZE];
		FThreadSafeCounter64 Materials;
		uint8 PadToAvoidContention2[PLATFORM_CACHE_LINE_SIZE];
	};

	const FDataOctreeMemory& GetCachedMemory() const { return CachedMemory; }
	const FDataOctreeMemory& GetDirtyMemory() const { return DirtyMemory; }
	
private:
	mutable FDataOctreeMemory CachedMemory{};
	mutable FDataOctreeMemory DirtyMemory{};
	
	template<typename>
	friend struct TVoxelDataOctreeLeafMemoryUsage;
};

class IVoxelData : public IVoxelDataOctreeMemory
{
public:
	const int32 Depth;
	const FVoxelIntBox WorldBounds;
	const bool bEnableMultiplayer;
	const bool bEnableUndoRedo;
	const TVoxelSharedRef<FVoxelGeneratorInstance> Generator;

	IVoxelData(
		int32 Depth,
		const FVoxelIntBox& WorldBounds,
		bool bEnableMultiplayer,
		bool bEnableUndoRedo,
		const TVoxelSharedRef<FVoxelGeneratorInstance>& Generator)
		: Depth(Depth)
		, WorldBounds(WorldBounds)
		, bEnableMultiplayer(bEnableMultiplayer)
		, bEnableUndoRedo(bEnableUndoRedo)
		, Generator(Generator)
	{
	}
};