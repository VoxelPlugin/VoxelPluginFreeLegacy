// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelEnums.h"
#include "VoxelContainers/VoxelStaticArray.h"

class IVoxelQueuedWork;

namespace EVoxelTaskType_DefaultPriorityCategories
{
	enum Type : int32
	{
		Min                            = 0,
		Max                            = 1000000,
		
		ChunksMeshing                  = 0,
		CollisionsChunksMeshing        = 1,
		VisibleChunksMeshing           = 10,
		VisibleCollisionsChunksMeshing = 100,
		CollisionCooking               = 100,
		FoliageBuild                   = 100,
		HISMBuild                      = 1000,
		AsyncEditFunctions             = 50,
		MeshMerge                      = 100000,
		RenderOctree                   = 1000000
	};
}

namespace EVoxelTaskType_DefaultPriorityOffsets
{
	enum Type : int32
	{
		ChunksMeshing                  = 0,
		CollisionsChunksMeshing        = 0,
		VisibleChunksMeshing           = 0,
		// By default, do collision cooking slightly before collision meshing, and foliage slightly after
		VisibleCollisionsChunksMeshing = 0,
		CollisionCooking               = +32,
		FoliageBuild                   = -32,
		HISMBuild                      = 0,
		AsyncEditFunctions             = 0,
		MeshMerge                      = 0,
		RenderOctree                   = 0
	};
}

class VOXEL_API FVoxelPool
{
public:
	static TVoxelSharedRef<FVoxelPool> Create(
		const TMap<EVoxelTaskType, int32>& PriorityCategories,
		const TMap<EVoxelTaskType, int32>& PriorityOffsets);

public:
	void QueueTask(IVoxelQueuedWork* Task);
	void QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks);
	
private:
	const TVoxelStaticArray<uint32, 256> PriorityCategories;
	const TVoxelStaticArray<uint32, 256> PriorityOffsets;

	explicit FVoxelPool(
		const TMap<EVoxelTaskType, int32>& PriorityCategories,
		const TMap<EVoxelTaskType, int32>& PriorityOffsets);

public:
	static void FixPriorityCategories(TMap<EVoxelTaskType, int32>& PriorityCategories);
	static void FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets);
};