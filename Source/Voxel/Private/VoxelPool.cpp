// Copyright 2021 Phyronnaz

#include "VoxelPool.h"
#include "VoxelThreadPool.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

DEFINE_UNIQUE_VOXEL_ID(FVoxelPoolId);
DEFINE_VOXEL_SUBSYSTEM_PROXY(UVoxelPoolSubsystemProxy);

void FVoxelPool::Create()
{
	Super::Create();
	
	auto FixedPriorityCategories = Settings.PriorityCategories;
	auto FixedPriorityOffsets = Settings.PriorityOffsets;
	FixPriorityCategories(FixedPriorityCategories);
	FixPriorityOffsets(FixedPriorityOffsets);
	
	for (int32 Index = 0; Index < 256; Index++)
	{
		PriorityCategories[Index] = FixedPriorityCategories.FindRef(EVoxelTaskType(Index));
		PriorityOffsets[Index] = FixedPriorityOffsets.FindRef(EVoxelTaskType(Index));
	}
}

void FVoxelPool::QueueTask(IVoxelQueuedWork* Task) const
{
	const EVoxelTaskType Type = Task->TaskType;
	GVoxelThreadPool->AddQueuedWorks(TVoxelStaticArray<IVoxelQueuedWork*, 1>{ Task }, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)], PoolId, Type);
}

void FVoxelPool::QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks) const
{
	GVoxelThreadPool->AddQueuedWorks(Tasks, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)], PoolId, Type);
}

void FVoxelPool::FixPriorityCategories(TMap<EVoxelTaskType, int32>& PriorityCategories)
{
	for (auto& It : PriorityCategories)
	{
		It.Value = FMath::Max(0, It.Value);
	}
#define FIX(Name) if (!PriorityCategories.Contains(EVoxelTaskType::Name)) PriorityCategories.Add(EVoxelTaskType::Name, EVoxelTaskType_DefaultPriorityCategories::Name);
	FIX(ChunksMeshing);
	FIX(CollisionsChunksMeshing);
	FIX(VisibleChunksMeshing);
	FIX(VisibleCollisionsChunksMeshing);
	FIX(CollisionCooking);
	FIX(FoliageBuild);
	FIX(HISMBuild);
	FIX(AsyncEditFunctions);
	FIX(MeshMerge);
	FIX(RenderOctree);
#undef FIX
}

void FVoxelPool::FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets)
{
#define FIX(Name) if (!PriorityOffsets.Contains(EVoxelTaskType::Name)) PriorityOffsets.Add(EVoxelTaskType::Name, EVoxelTaskType_DefaultPriorityOffsets::Name);
	FIX(ChunksMeshing);
	FIX(VisibleChunksMeshing);
	FIX(CollisionsChunksMeshing);
	FIX(VisibleCollisionsChunksMeshing);
	FIX(CollisionCooking);
	FIX(FoliageBuild);
	FIX(HISMBuild);
	FIX(AsyncEditFunctions);
	FIX(RenderOctree);
	FIX(MeshMerge);
#undef FIX
}