// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelPriorityHandler.h"
#include "Misc/IQueuedWork.h"

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
	}
	UE_NONCOPYABLE(IVoxelQueuedWork);
	
	FORCEINLINE uint32 GetPriority() const
	{
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
	
	friend class FVoxelThreadPool;
};