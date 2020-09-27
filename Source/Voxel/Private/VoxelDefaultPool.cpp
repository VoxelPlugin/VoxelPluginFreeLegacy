// Copyright 2020 Phyronnaz

#include "VoxelDefaultPool.h"
#include "VoxelThreadPool.h"
#include "VoxelQueuedWork.h"
#include "Misc/QueuedThreadPool.h"
#include "VoxelMinimal.h"

FVoxelDefaultPool::FVoxelDefaultPool(
	int32 ThreadCount,
	bool bConstantPriorities,
	const TMap<EVoxelTaskType, int32>& InPriorityCategories,
	const TMap<EVoxelTaskType, int32>& InPriorityOffsets)
	: Pool(FVoxelQueuedThreadPool::Create(FVoxelQueuedThreadPoolSettings(
		FString::Printf(TEXT("Voxel Pool %llu"), UNIQUE_ID()),
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

void FVoxelDefaultPool::FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets)
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