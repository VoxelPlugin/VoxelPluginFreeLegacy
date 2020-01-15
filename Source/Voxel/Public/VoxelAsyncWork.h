// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelQueuedWork.h"

class VOXEL_API FVoxelAsyncWork : public IVoxelQueuedWork
{
public:
	inline FVoxelAsyncWork(FName Name, double PriorityDuration, bool bAutoDelete = false)
		: IVoxelQueuedWork(Name, PriorityDuration)
		, bAutodelete(bAutoDelete)
	{
	}
	~FVoxelAsyncWork() override;

	//~ Begin IVoxelQueuedWork Interface
	virtual void DoThreadedWork() override final;
	virtual void Abandon() override final;
	//~ End IVoxelQueuedWork Interface

	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() = 0;
	virtual void PostDoWork() {} // Will be called when IsDone is true
	//~ End FVoxelAsyncWork Interface

	// @return: IsDone and PostDoWork was called
	bool CancelAndAutodelete();
	
	bool IsDone() const
	{
		return IsDoneCounter.GetValue() > 0;
	}
	bool WasAbandoned() const
	{
		return WasAbandonedCounter.GetValue() > 0;
	}

protected:
	bool IsCanceled() const
	{
		return CanceledCounter.GetValue() > 0;
	}
	void SetIsDone(bool bIsDone)
	{
		check(!bAutodelete);
		IsDoneCounter.Set(bIsDone ? 1 : 0);
	}

private:
	FThreadSafeCounter IsDoneCounter;
	FCriticalSection DoneSection;
	
	FThreadSafeCounter CanceledCounter;
	bool bAutodelete = false;

	FThreadSafeCounter WasAbandonedCounter;
};

class VOXEL_API FVoxelAsyncWorkWithWait : public FVoxelAsyncWork
{
public:
	FVoxelAsyncWorkWithWait(FName Name, double PriorityDuration, bool bAutoDelete = false);
	virtual ~FVoxelAsyncWorkWithWait() override;

	//~ Begin IVoxelQueuedWork Interface
	virtual void PostDoWork() override final;
	//~ End IVoxelQueuedWork Interface

	//~ Begin FVoxelAsyncWorkWithWait Interface
	virtual void PostDoWorkBeforeTrigger() {};
	//~ End FVoxelAsyncWorkWithWait Interface

	void WaitForCompletion();

private:
	FEvent* DoneEvent;
};