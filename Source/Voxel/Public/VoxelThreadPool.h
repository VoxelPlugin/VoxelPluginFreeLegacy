// Copyright 2019 Phyronnaz

#pragma once

#include <queue>

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "GenericPlatform/GenericPlatformAffinity.h"

class IVoxelQueuedWork
{
public:
	const uint64 Priority;
	const FName Name;

	IVoxelQueuedWork(uint64 Priority, const FName& Name)
		: Priority(Priority)
		, Name(Name)
	{
	}
	virtual ~IVoxelQueuedWork() {}
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
};

class FVoxelQueuedWorkCompare
{
public:
	inline bool operator() (const IVoxelQueuedWork* A, const IVoxelQueuedWork* B)
	{
		return A->Priority < B->Priority;
	}
};

class VOXEL_API FVoxelQueuedThread : public FRunnable
{
protected:
	/** The event that tells the thread there is work to do. */
	FEvent* DoWorkEvent = nullptr;

	/** If true, the thread should exit. */
	volatile int32 TimeToDie = 0;

	/** The work this thread is doing. */
	IVoxelQueuedWork* volatile QueuedWork = nullptr;

	/** The pool this thread belongs to. */
	class FVoxelQueuedThreadPool* OwningThreadPool = nullptr;

	/** My Thread  */
	FRunnableThread* Thread = nullptr;

	/**
	 * The real thread entry point. It waits for work events to be queued. Once
	 * an event is queued, it executes it and goes back to waiting.
	 */
	virtual uint32 Run() override;

public:
	FVoxelQueuedThread() = default;

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
	std::priority_queue<IVoxelQueuedWork*, std::vector<IVoxelQueuedWork*>, FVoxelQueuedWorkCompare> QueuedWork;

	/** The thread pool to dole work out to. */
	TArray<FVoxelQueuedThread*> QueuedThreads;

	/** All threads in the pool. */
	TArray<FVoxelQueuedThread*> AllThreads;

	/** The synchronization object used to protect access to the queued work. */
	FCriticalSection* SyncQueue = nullptr;

	/** If true, indicates the destruction process has taken place. */
	bool TimeToDie = false;

public:

	static FVoxelQueuedThreadPool* Allocate() { return new FVoxelQueuedThreadPool(); }

	/** Default constructor. */
	FVoxelQueuedThreadPool() = default;

	/** Virtual destructor (cleans up the synchronization objects). */
	~FVoxelQueuedThreadPool();

	bool Create(uint32 InNumQueuedThreads, uint32 StackSize = (32 * 1024), EThreadPriority ThreadPriority = TPri_Normal);

	void Destroy();

	int32 GetNumQueuedJobs() const;

	//int32 GetNumQueuedJobs() const;
	int32 GetNumThreads() const;

	void AddQueuedWork(IVoxelQueuedWork* InQueuedWork);

	IVoxelQueuedWork* ReturnToPoolOrGetNextJob(FVoxelQueuedThread* InQueuedThread);
};

DECLARE_DELEGATE(FVoxelAsyncWorkCallback);

class VOXEL_API FVoxelAsyncWork : public IVoxelQueuedWork
{
public:
	FVoxelAsyncWork(const FName& Name, uint64 Priority, FVoxelAsyncWorkCallback Callback = FVoxelAsyncWorkCallback(), bool bAutodelete = false);
	~FVoxelAsyncWork() override;

	virtual void DoThreadedWork() final;
	virtual void Abandon() final;

	virtual void DoWork() = 0;

	inline bool IsDone() const { return IsDoneCounter.GetValue() > 0; }

	void WaitForCompletion();
	void CancelAndAutodelete();

protected:
	inline bool IsCanceled() const { return bCanceled.GetValue() > 0; }

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;

	FVoxelAsyncWorkCallback Callback;
	uint64 CallbackInfo;
	
	FThreadSafeCounter bCanceled;
	bool bAutodelete = false;
};

