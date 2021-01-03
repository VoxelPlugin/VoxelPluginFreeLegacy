// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelPriorityHandler.h"
#include "Misc/IQueuedWork.h"

DECLARE_UNIQUE_VOXEL_ID(FVoxelPoolId);

#define ENABLE_VOXEL_QUEUED_WORK_CHECKS 1

class IVoxelQueuedWork : public IQueuedWork
{
public:
	enum class EPriority
	{
		Null,
		InvokersDistance
	};
	
	const FName Name;
	const EVoxelTaskType TaskType;
	const EPriority Priority;

	FORCEINLINE IVoxelQueuedWork(FName Name, EVoxelTaskType TaskType, EPriority Priority)
		: Name(Name)
		, TaskType(TaskType)
		, Priority(Priority)
	{
		Register();
	}
	~IVoxelQueuedWork()
	{
		Unregister();
	}
	UE_NONCOPYABLE(IVoxelQueuedWork);
	
	FORCEINLINE uint32 GetPriority() const
	{
		CheckIsValidLowLevel();
		
		if (Priority == EPriority::Null)
		{
			return 0;
		}
		else
		{
			return PriorityHandler.GetPriority();
		}
	}

	// If true, Abandon will be called instead of DoThreadedWork if possible
	bool ShouldAbandon() const { return bShouldAbandon; }

protected:
	bool bShouldAbandon = false;
	FVoxelPriorityHandler PriorityHandler;

private:
	int32 PriorityOffset = 0;
	FVoxelPoolId PoolId;
	
	friend class FVoxelThread;
	friend class FVoxelThreadPool;

public:
#if ENABLE_VOXEL_QUEUED_WORK_CHECKS
	void CheckIsValidLowLevel() const;
#else
	FORCEINLINE void CheckIsValidLowLevel() const {}
#endif

private:
#if ENABLE_VOXEL_QUEUED_WORK_CHECKS
	void Register();
	void Unregister();
	
	struct FCheckData
	{
		FCriticalSection Section;
		TSet<const IVoxelQueuedWork*> ValidWorks;
		TMap<const IVoxelQueuedWork*, TArray<FName>> WorkNames;
	};
	static FCheckData CheckData;
#else
	FORCEINLINE void Register() {}
	FORCEINLINE void Unregister() {}
#endif
};