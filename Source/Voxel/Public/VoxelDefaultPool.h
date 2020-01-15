// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "IVoxelPool.h"

class VOXEL_API FVoxelDefaultPool : public IVoxelPool
{
public:
	static TVoxelSharedRef<FVoxelDefaultPool> Create(
		int32 ThreadCount,
		bool bConstantPriorities,
		const TMap<EVoxelTaskType, int32>& PriorityCategories,
		const TMap<EVoxelTaskType, int32>& PriorityOffsets);
	virtual ~FVoxelDefaultPool();

public:
	//~ Begin IVoxelPool Interface
	virtual void QueueTask(EVoxelTaskType Type, IVoxelQueuedWork* Task) override;
	virtual void QueueTasks(EVoxelTaskType Type, const TArray<IVoxelQueuedWork*>& Tasks) override;

	virtual int32 GetNumTasks() const override;
	//~ End IVoxelPool Interface
	
private:
	const TVoxelSharedRef<FVoxelQueuedThreadPool> Pool;
	const TStaticArray<uint32, 256> PriorityCategories;
	const TStaticArray<uint32, 256> PriorityOffsets;

	explicit FVoxelDefaultPool(
		int32 ThreadCount,
		bool bConstantPriorities,
		const TMap<EVoxelTaskType, int32>& PriorityCategories,
		const TMap<EVoxelTaskType, int32>& PriorityOffsets);

public:
	static void FixPriorityCategories(TMap<EVoxelTaskType, int32>& PriorityCategories);
	static void FixPriorityOffsets(TMap<EVoxelTaskType, int32>& PriorityOffsets);
};
