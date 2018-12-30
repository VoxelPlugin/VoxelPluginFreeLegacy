// Copyright 2018 Phyronnaz

#pragma once

#include <queue>

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "GenericPlatform/GenericPlatformAffinity.h"

class IVoxelQueuedWork
{
public:

	/**
	 * This is where the real thread work is done. All work that is done for
	 * this queued object should be done from within the call to this function.
	 */
	virtual void DoThreadedWork() = 0;

	/**
	 * Tells the queued work that it is being abandoned so that it can do
	 * per object clean up as needed. This will only be called if it is being
	 * abandoned before completion. NOTE: This requires the object to delete
	 * itself using whatever heap it was allocated in.
	 */
	virtual void Abandon() = 0;

	virtual uint64 GetPriority() const = 0;

public:

	/**
	 * Virtual destructor so that child implementations are guaranteed a chance
	 * to clean up any resources they allocated.
	 */
	virtual ~IVoxelQueuedWork() {}
};

// Needed because of priorities queries
struct FVoxelQueuedWorkPtr
{
	IVoxelQueuedWork* Work;
	const TSet<IVoxelQueuedWork*>* ValidQueuedWork;

	FVoxelQueuedWorkPtr(IVoxelQueuedWork* InWork, const TSet<IVoxelQueuedWork*>& InValidQueuedWork)
		: Work(InWork)
		, ValidQueuedWork(&InValidQueuedWork)
	{
	}

	uint64 GetPriority() const
	{
		if (ValidQueuedWork->Contains(Work))
		{
			return Work->GetPriority();
		}
		else
		{
			return MAX_uint64; // Put invalid jobs on top to avoid too much memory usage
		}
	}
};

class FVoxelQueuedWorkCompare
{
public:
	bool operator() (const FVoxelQueuedWorkPtr& A, const FVoxelQueuedWorkPtr& B)
	{
		return A.GetPriority() < B.GetPriority();
	}
};

class VOXEL_API FVoxelQueuedThread : public FRunnable
{
protected:

	/** The event that tells the thread there is work to do. */
	FEvent * DoWorkEvent;

	/** If true, the thread should exit. */
	volatile int32 TimeToDie;

	/** The work this thread is doing. */
	IVoxelQueuedWork* volatile QueuedWork;

	/** The pool this thread belongs to. */
	class FVoxelQueuedThreadPool* OwningThreadPool;

	/** My Thread  */
	FRunnableThread* Thread;

	/**
	 * The real thread entry point. It waits for work events to be queued. Once
	 * an event is queued, it executes it and goes back to waiting.
	 */
	virtual uint32 Run() override;

public:

	/** Default constructor **/
	FVoxelQueuedThread();

	/**
	 * Creates the thread with the specified stack size and creates the various
	 * events to be able to communicate with it.
	 *
	 * @param InPool The thread pool interface used to place this thread back into the pool of available threads when its work is done
	 * @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
	 * @param ThreadPriority priority of new thread
	 * @return True if the thread and all of its initialization was successful, false otherwise
	 */
	bool Create(class FVoxelQueuedThreadPool* InPool, uint32 InStackSize = 0, EThreadPriority ThreadPriority = TPri_Normal);

	/**
	 * Tells the thread to exit. If the caller needs to know when the thread
	 * has exited, it should use the bShouldWait value and tell it how long
	 * to wait before deciding that it is deadlocked and needs to be destroyed.
	 * NOTE: having a thread forcibly destroyed can cause leaks in TLS, etc.
	 *
	 * @return True if the thread exited graceful, false otherwise
	 */
	bool KillThread();

	/**
	 * Tells the thread there is work to be done. Upon completion, the thread
	 * is responsible for adding itself back into the available pool.
	 *
	 * @param InQueuedWork The queued work to perform
	 */
	void DoWork(IVoxelQueuedWork* InQueuedWork);

};

class VOXEL_API FVoxelQueuedThreadPool
{
protected:

	/** The work queue to pull from. */
	std::priority_queue<FVoxelQueuedWorkPtr, std::vector<FVoxelQueuedWorkPtr>, FVoxelQueuedWorkCompare> QueuedWork;
	TSet<IVoxelQueuedWork*> ValidQueuedWork;

	/** The thread pool to dole work out to. */
	TArray<FVoxelQueuedThread*> QueuedThreads;

	/** All threads in the pool. */
	TArray<FVoxelQueuedThread*> AllThreads;

	/** The synchronization object used to protect access to the queued work. */
	FCriticalSection* SynchQueue;

	/** If true, indicates the destruction process has taken place. */
	bool TimeToDie;

public:

	static FVoxelQueuedThreadPool* Allocate() { return new FVoxelQueuedThreadPool(); }

	/** Default constructor. */
	FVoxelQueuedThreadPool();

	/** Virtual destructor (cleans up the synchronization objects). */
	~FVoxelQueuedThreadPool();

	bool Create(uint32 InNumQueuedThreads, uint32 StackSize = (32 * 1024), EThreadPriority ThreadPriority = TPri_Normal);

	void Destroy();

	int32 GetNumQueuedJobs() const;

	//int32 GetNumQueuedJobs() const;
	int32 GetNumThreads() const;

	void AddQueuedWork(IVoxelQueuedWork* InQueuedWork);

	bool RetractQueuedWork(IVoxelQueuedWork* InQueuedWork);

	IVoxelQueuedWork* ReturnToPoolOrGetNextJob(FVoxelQueuedThread* InQueuedThread);
};

class IVoxelAsyncCallback
{
public:
	virtual void OnCallback() = 0;
};

class VOXEL_API FVoxelAsyncWork : public IVoxelQueuedWork
{
public:
	FVoxelAsyncWork(IVoxelAsyncCallback* Callback = nullptr, bool bAutodelete = false);
	~FVoxelAsyncWork() override;

	virtual void DoWork() = 0;

	virtual void DoThreadedWork() final;
	virtual void Abandon() final;

	bool IsDone() const;
	void WaitForCompletion();

	void CancelAndAutodelete();

protected:
	bool IsCanceled() const { return bCanceled; }

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;

	IVoxelAsyncCallback* Callback;
	bool bAutodelete = false;
	bool bCanceled = false;
};

