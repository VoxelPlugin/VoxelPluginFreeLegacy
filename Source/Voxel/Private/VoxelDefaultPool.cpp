// Copyright 2020 Phyronnaz

#include "VoxelDefaultPool.h"
#include "VoxelThreadPool.h"
#include "VoxelQueuedWork.h"
#include "Misc/QueuedThreadPool.h"
#include "VoxelGlobals.h"

FVoxelDefaultPool::FVoxelDefaultPool(
	int32 ThreadCount,
	bool bConstantPriorities,
	const TMap<EVoxelTaskType, int32>& InPriorityCategories,
	const TMap<EVoxelTaskType, int32>& InPriorityOffsets)
	: Pool(FVoxelQueuedThreadPool::Create(FVoxelQueuedThreadPoolSettings(
		FString::Printf(TEXT("Default Pool %llu"), UNIQUE_ID()),
		ThreadCount,
		1024 * 1024,
		EThreadPriority::TPri_Normal,
		bConstantPriorities)))
{
	for (int32 Index = 0; Index < 256; Index++)
	{
		const_cast<TStaticArray<uint32, 256>&>(PriorityCategories)[Index] = InPriorityCategories.FindRef(EVoxelTaskType(Index));
		const_cast<TStaticArray<uint32, 256>&>(PriorityOffsets)[Index] = InPriorityOffsets.FindRef(EVoxelTaskType(Index));
	}
}

FVoxelDefaultPool::~FVoxelDefaultPool()
{
}

TVoxelSharedRef<FVoxelDefaultPool> FVoxelDefaultPool::Create(
	int32 ThreadCount,
	bool bConstantPriorities,
	const TMap<EVoxelTaskType, int32>& PriorityCategories,
	const TMap<EVoxelTaskType, int32>& PriorityOffsets)
{
	LOG_VOXEL(Log, TEXT("Creating pool with %d threads"), ThreadCount);
	if (!ensureMsgf(ThreadCount >= 1, TEXT("Invalid MeshThreadCount: %d"), ThreadCount))
	{
		ThreadCount = 1;
	}
	
	auto FixedPriorityCategories = PriorityCategories;
	auto FixedPriorityOffsets = PriorityOffsets;
	FixPriorityCategories(FixedPriorityCategories);
	FixPriorityOffsets(FixedPriorityOffsets);
	
	return MakeShareable(new FVoxelDefaultPool(
		ThreadCount,
		bConstantPriorities,
		FixedPriorityCategories,
		FixedPriorityOffsets));
}

void FVoxelDefaultPool::QueueTask(EVoxelTaskType Type, IVoxelQueuedWork* Task)
{
	Pool->AddQueuedWork(Task, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)]);
}

void FVoxelDefaultPool::QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks)
{
	Pool->AddQueuedWorks(Tasks, PriorityCategories[uint8(Type)], PriorityOffsets[uint8(Type)]);
}

int32 FVoxelDefaultPool::GetNumTasks() const
{
	return Pool->GetNumPendingWorks();
}

void FVoxelDefaultPool::FixPriorityCategories(TMap<EVoxelTaskType, int32>& PriorityCategories)
{
	for (auto& It : PriorityCategories)
	{
		It.Value = FMath::Max(0, It.Value);
	}
#define FIX(Name, DefaultValue) if (!PriorityCategories.Contains(EVoxelTaskType::Name)) PriorityCategories.Add(EVoxelTaskType::Name, DefaultValue);
	FIX(ChunksMeshing, 0);
	FIX(CollisionsChunksMeshing, 1);
	FIX(VisibleChunksMeshing, 10);
	FIX(VisibleCollisionsChunksMeshing, 100);
	FIX(CollisionCooking, 100);
	FIX(FoliageBuild, 100);
	FIX(HISMBuild, 1000);
	FIX(AsyncEditFunctions, 50);
	FIX(MeshMerge, 100000);
	FIX(RenderOctree, 1000000);
#undef FIX
}

void FVoxelDefaultPool::FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets)
{
#define FIX(Name, DefaultValue) if (!PriorityOffsets.Contains(EVoxelTaskType::Name)) PriorityOffsets.Add(EVoxelTaskType::Name, DefaultValue);
	FIX(ChunksMeshing, 0);
	FIX(VisibleChunksMeshing, 0);
	FIX(CollisionsChunksMeshing, 0);
	FIX(VisibleCollisionsChunksMeshing, 0);
	FIX(CollisionCooking, 32);
	FIX(FoliageBuild, -32);
	FIX(HISMBuild, 0);
	FIX(AsyncEditFunctions, 0);
	FIX(RenderOctree, 0);
	FIX(MeshMerge, 0);
#undef FIX
}