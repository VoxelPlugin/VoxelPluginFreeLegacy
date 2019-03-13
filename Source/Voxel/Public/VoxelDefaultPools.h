// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelPool.h"

class VOXEL_API FVoxelDefaultPool : public IVoxelPool
{
public:
	static void CreateGlobalPool(int MeshThreadCount);
	static TSharedRef<FVoxelDefaultPool> Create(int MeshThreadCount);

public:
	virtual void QueueAsyncEditTask(IQueuedWork* Work) override;
	virtual void QueueCollisionTask(IVoxelQueuedWork* Work) override;
	virtual void QueueCacheTask(IVoxelQueuedWork* Work) override;
	virtual void QueueMeshingTask(IVoxelQueuedWork* Work) override;
	virtual void QueueOctreeBuildTask(IQueuedWork* Work) override;

private:
	FVoxelQueuedThreadPool* const MeshPool;
	FQueuedThreadPool* const OctreeBuilderPool;
	FQueuedThreadPool* const AsyncTasksPool;

	FVoxelDefaultPool(int MeshThreadCount);

public:
	virtual ~FVoxelDefaultPool();
};