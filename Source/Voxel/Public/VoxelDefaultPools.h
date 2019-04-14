// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelPool.h"

class VOXEL_API FVoxelDefaultPool : public IVoxelPool
{
public:
	static void CreateGlobalPool(int32 MeshThreadCount);
	static TSharedRef<FVoxelDefaultPool> Create(int32 MeshThreadCount);

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

	FVoxelDefaultPool(int32 MeshThreadCount);

public:
	virtual ~FVoxelDefaultPool();
};