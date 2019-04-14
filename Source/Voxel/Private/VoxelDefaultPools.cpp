// Copyright 2019 Phyronnaz

#include "VoxelDefaultPools.h"
#include "VoxelThreadPool.h"
#include "Misc/QueuedThreadPool.h"

FVoxelDefaultPool::FVoxelDefaultPool(int32 MeshThreadCount)
	: MeshPool(FVoxelQueuedThreadPool::Allocate())
	, OctreeBuilderPool(FQueuedThreadPool::Allocate())
	, AsyncTasksPool(FQueuedThreadPool::Allocate())
{
	MeshPool->Create(MeshThreadCount, 1024 * 1024);
	OctreeBuilderPool->Create(1, 1024 * 1024);
	AsyncTasksPool->Create(1, 1024 * 1024);
}

FVoxelDefaultPool::~FVoxelDefaultPool()
{
	delete MeshPool;
	delete OctreeBuilderPool;
	delete AsyncTasksPool;
}

void FVoxelDefaultPool::CreateGlobalPool(int32 MeshThreadCount)
{
	IVoxelPool::SetGlobalVoxelPool(Create(MeshThreadCount));
}

TSharedRef<FVoxelDefaultPool> FVoxelDefaultPool::Create(int32 MeshThreadCount)
{
	if (!ensureMsgf(MeshThreadCount >= 1, TEXT("Invalid MeshThreadCount: %d"), MeshThreadCount))
	{
		MeshThreadCount = 1;
	}
	return MakeShareable(new FVoxelDefaultPool(MeshThreadCount));
}

void FVoxelDefaultPool::QueueAsyncEditTask(IQueuedWork* Work)
{
	AsyncTasksPool->AddQueuedWork(Work);
}

void FVoxelDefaultPool::QueueCollisionTask(IVoxelQueuedWork* Work)
{
	MeshPool->AddQueuedWork(Work);
}

void FVoxelDefaultPool::QueueCacheTask(IVoxelQueuedWork* Work)
{
	MeshPool->AddQueuedWork(Work);
}

void FVoxelDefaultPool::QueueMeshingTask(IVoxelQueuedWork* Work)
{
	MeshPool->AddQueuedWork(Work);
}

void FVoxelDefaultPool::QueueOctreeBuildTask(IQueuedWork* Work)
{
	OctreeBuilderPool->AddQueuedWork(Work);
}