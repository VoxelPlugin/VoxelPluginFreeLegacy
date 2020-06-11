// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelQueuedWork.h"

class VOXEL_API FVoxelAsyncWork : public IVoxelQueuedWork
{
public:
	FORCEINLINE FVoxelAsyncWork(FName Name, double PriorityDuration, bool bAutoDelete = false)
		: IVoxelQueuedWork(Name, PriorityDuration)
		, bAutodelete(bAutoDelete)
	{
	}

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
	// Important: do not allow public delete
	~FVoxelAsyncWork() override;
	
	bool IsCanceled() const
	{
		return CanceledCounter.GetValue() > 0;
	}
	void SetIsDone(bool bIsDone)
	{
		check(!bAutodelete);
		IsDoneCounter.Set(bIsDone ? 1 : 0);
	}

	void WaitForDoThreadedWorkToExit();

private:
	struct FSafeCriticalSection
	{
		FCriticalSection Section;
		FThreadSafeCounter IsLocked;

		FORCEINLINE void Lock()
		{
			Section.Lock();
			ensure(IsLocked.Set(1) == 0);
		}
		FORCEINLINE void Unlock()
		{
			ensure(IsLocked.Set(0) == 1);
			Section.Unlock();
		}
	};
	
	FThreadSafeCounter IsDoneCounter;
	FSafeCriticalSection DoneSection;
	
	FThreadSafeCounter CanceledCounter;
	bool bAutodelete = false;

	FThreadSafeCounter WasAbandonedCounter;
};

template<typename T>
struct TVoxelAsyncWorkDelete
{
	void operator()(T* Ptr) const
	{
		if (Ptr)
		{
			Ptr->WaitForDoThreadedWorkToExit();
		}
		delete Ptr;
	}
};

class VOXEL_API FVoxelAsyncWorkWithWait : public FVoxelAsyncWork
{
public:
	FVoxelAsyncWorkWithWait(FName Name, double PriorityDuration, bool bAutoDelete = false);

	//~ Begin IVoxelQueuedWork Interface
	virtual void PostDoWork() override final;
	//~ End IVoxelQueuedWork Interface

	//~ Begin FVoxelAsyncWorkWithWait Interface
	virtual void PostDoWorkBeforeTrigger() {};
	//~ End FVoxelAsyncWorkWithWait Interface

	void WaitForCompletion();

protected:
	// Important: do not allow public delete
	virtual ~FVoxelAsyncWorkWithWait() override;

private:
	FEvent* DoneEvent;
};