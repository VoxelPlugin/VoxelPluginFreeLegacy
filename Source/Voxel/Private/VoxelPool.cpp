// Copyright 2020 Phyronnaz

#include "VoxelPool.h"
#include "VoxelMinimal.h"
#include "VoxelThreadPool.h"

FVoxelPool::FVoxelPool(
	const TMap<EVoxelTaskType, int32>& InPriorityCategories,
	const TMap<EVoxelTaskType, int32>& InPriorityOffsets)
{
	for (int32 Index = 0; Index < 256; Index++)
	{
		const_cast<TVoxelStaticArray<uint32, 256>&>(PriorityCategories)[Index] = InPriorityCategories.FindRef(EVoxelTaskType(Index));
		const_cast<TVoxelStaticArray<uint32, 256>&>(PriorityOffsets)[Index] = InPriorityOffsets.FindRef(EVoxelTaskType(Index));
	}
}

TVoxelSharedRef<FVoxelPool> FVoxelPool::Create(
	const TMap<EVoxelTaskType, int32>& PriorityCategories,
	const TMap<EVoxelTaskType, int32>& PriorityOffsets)
{
	auto FixedPriorityCategories = PriorityCategories;
	auto FixedPriorityOffsets = PriorityOffsets;
	FixPriorityCategories(FixedPriorityCategories);
	FixPriorityOffsets(FixedPriorityOffsets);
	
	return MakeShareable(new FVoxelPool(
		FixedPriorityCategories,
		FixedPriorityOffsets));
}

void FVoxelPool::QueueTask(IVoxelQueuedWork* Task)
{
	const EVoxelTaskType Type = Task->TaskType;
	GVoxelThreadPool->AddQueuedWorks(TVoxelStaticArray<IVoxelQueuedWork*, 1>{ Task }, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)], Type);
}

void FVoxelPool::QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks)
{
	GVoxelThreadPool->AddQueuedWorks(Tasks, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)], Type);
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