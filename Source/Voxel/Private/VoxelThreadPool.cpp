// Copyright 2020 Phyronnaz

#include "VoxelThreadPool.h"
#include "VoxelQueuedWork.h"
#include "VoxelMinimal.h"
#include "IVoxelPool.h"

#include "HAL/Event.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Async/TaskGraphInterfaces.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("VoxelThreadPoolDummyCounter"), STAT_VoxelThreadPoolDummyCounter, STATGROUP_ThreadPoolAsyncTasks);
DECLARE_DWORD_COUNTER_STAT(TEXT("Recomputed Voxel Tasks Priorities"), STAT_RecomputedVoxelTasksPriorities, STATGROUP_VoxelCounters);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQueuedThreadPoolStats& FVoxelQueuedThreadPoolStats::Get()
{
	static FVoxelQueuedThreadPoolStats Stats;
	return Stats;
}

void FVoxelQueuedThreadPoolStats::Report(FName Name, double Time)
{
	FScopeLock Lock(&Section);
	Times.FindOrAdd(Name) += Time;
}

void FVoxelQueuedThreadPoolStats::LogTimes() const
{
	FScopeLock Lock(&Section);
	LOG_VOXEL(Log, TEXT("#############################################"));
	LOG_VOXEL(Log, TEXT("########## Voxel Thread Pool Stats ##########"));
	LOG_VOXEL(Log, TEXT("#############################################"));
	for (const auto& It : Times)
	{
		LOG_VOXEL(Log, TEXT("%s: %fs"), *It.Key.ToString(), It.Value);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FScopeLockWithStats
{
public:
	FScopeLockWithStats(FCriticalSection& InSynchObject)
		: SynchObject(InSynchObject)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Lock");
		SynchObject.Lock();
	}
	~FScopeLockWithStats()
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Unlock");
		SynchObject.Unlock();
	}

private:
	FCriticalSection& SynchObject;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelQueuedThread : public FRunnable
{
public:
	const FString ThreadName;
	FVoxelQueuedThreadPool* const ThreadPool;
	/** The event that tells the thread there is work to do. */
	FEvent* const DoWorkEvent;

	FVoxelQueuedThread(FVoxelQueuedThreadPool* Pool, const FString& ThreadName, uint32 StackSize, EThreadPriority ThreadPriority);
	~FVoxelQueuedThread();

	//~ Begin FRunnable Interface
	virtual uint32 Run() override;
	//~ End FRunnable Interface

private:
	/** If true, the thread should exit. */
	FThreadSafeBool TimeToDie;
	/** The work this thread is doing. */
	TAtomic<IVoxelQueuedWork*> QueuedWork;

	const TUniquePtr<FRunnableThread> Thread;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQueuedThread::FVoxelQueuedThread(FVoxelQueuedThreadPool* Pool, const FString& ThreadName, uint32 StackSize, EThreadPriority ThreadPriority)
	: ThreadName(ThreadName)
	, ThreadPool(Pool)
	, DoWorkEvent(FPlatformProcess::GetSynchEventFromPool()) // Create event BEFORE thread
	, TimeToDie(false) // BEFORE creating thread
	, QueuedWork(nullptr)
	, Thread(FRunnableThread::Create(this, *ThreadName, StackSize, ThreadPriority, FPlatformAffinity::GetPoolThreadMask()))
{
	check(Thread.IsValid());
}

FVoxelQueuedThread::~FVoxelQueuedThread()
{
	// Tell the thread it needs to die
	TimeToDie = true;
	// Trigger the thread so that it will come out of the wait state if
	// it isn't actively doing work
	DoWorkEvent->Trigger();
	// If waiting was specified, wait the amount of time. If that fails,
	// brute force kill that thread. Very bad as that might leak.
	Thread->WaitForCompletion();
	// Clean up the event
	FPlatformProcess::ReturnSynchEventToPool(DoWorkEvent);
}

uint32 FVoxelQueuedThread::Run()
{
	while (!TimeToDie)
	{
		// This will force sending the stats packet from the previous frame.
		SET_DWORD_STAT(STAT_VoxelThreadPoolDummyCounter, 0);
		// We need to wait for shorter amount of time
		bool bContinueWaiting = true;
		while (bContinueWaiting)
		{
			VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER("FVoxelQueuedThread::Run.WaitForWork");
			
			// Wait for some work to do
			bContinueWaiting = !DoWorkEvent->Wait(10);
		}

		if (!TimeToDie)
		{
			IVoxelQueuedWork* LocalQueuedWork = ThreadPool->ReturnToPoolOrGetNextJob(this);

			while (LocalQueuedWork)
			{
				const FName Name = LocalQueuedWork->Name;
				
				const double StartTime = FPlatformTime::Seconds();
				
				LocalQueuedWork->DoThreadedWork();
				// IMPORTANT: LocalQueuedWork should be considered as deleted after this line
				
				const double EndTime = FPlatformTime::Seconds();

				FVoxelQueuedThreadPoolStats::Get().Report(Name, EndTime - StartTime);
				
				LocalQueuedWork = ThreadPool->ReturnToPoolOrGetNextJob(this);
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelQueuedThreadPoolSettings::FVoxelQueuedThreadPoolSettings(
	const FString& PoolName, 
	uint32 NumThreads, 
	uint32 StackSize, 
	EThreadPriority ThreadPriority, 
	bool bConstantPriorities)
	: PoolName(PoolName)
	, NumThreads(NumThreads)
	, StackSize(StackSize)
	, ThreadPriority(ThreadPriority)
	, bConstantPriorities(bConstantPriorities)
{
}

inline TArray<TUniquePtr<FVoxelQueuedThread>> CreateThreads(FVoxelQueuedThreadPool* Pool)
{
	TRACE_THREAD_GROUP_SCOPE("VoxelThreadPool");

	auto& Settings = Pool->Settings;
	const uint32 NumThreads = Settings.NumThreads;
	
	TArray<TUniquePtr<FVoxelQueuedThread>> Threads;
	Threads.Reserve(NumThreads);
	for (uint32 ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++)
	{
		const FString Name = FString::Printf(TEXT("%s Thread %d"), *Settings.PoolName, ThreadIndex);
		Threads.Add(MakeUnique<FVoxelQueuedThread>(Pool, Name, Settings.StackSize, Settings.ThreadPriority));
	}
	return Threads;
}

FVoxelQueuedThreadPool::FVoxelQueuedThreadPool(const FVoxelQueuedThreadPoolSettings& Settings)
	: Settings(Settings)
	, AllThreads(CreateThreads(this))
{
	QueuedThreads.Reserve(Settings.NumThreads);
	for (auto& Thread : AllThreads) 
	{
		QueuedThreads.Add(Thread.Get());
	}
}

TVoxelSharedRef<FVoxelQueuedThreadPool> FVoxelQueuedThreadPool::Create(const FVoxelQueuedThreadPoolSettings& Settings)
{
	const auto Pool = TVoxelSharedRef<FVoxelQueuedThreadPool>(new FVoxelQueuedThreadPool(Settings));

	TFunction<void()> ShutdownCallback = [WeakPool = MakeVoxelWeakPtr(Pool)]()
	{
		auto PoolPtr = WeakPool.Pin();
		if (PoolPtr.IsValid())
		{
			PoolPtr->AbandonAllTasks();
		}
	};
	FTaskGraphInterface::Get().AddShutdownCallback(ShutdownCallback);

	return Pool;
}

FVoxelQueuedThreadPool::~FVoxelQueuedThreadPool()
{
	if (!TimeToDie)
	{
		AbandonAllTasks();
	}
}

inline uint32 AddPriorityOffset(uint32 Priority, int32 PriorityOffset)
{
	return FMath::Clamp<int64>(int64(Priority) + PriorityOffset, MIN_uint32, MAX_uint32);
}

FORCEINLINE void FVoxelQueuedThreadPool::FQueuedWorkInfo::RecomputePriority(double Time)
{
	Priority = AddPriorityOffset(Work->GetPriority(), PriorityOffset);
	NextPriorityUpdateTime = Time + Work->PriorityDuration;
}

void FVoxelQueuedThreadPool::AddQueuedWork(IVoxelQueuedWork* InQueuedWork, uint32 PriorityCategory, int32 PriorityOffset)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(IsInGameThread());
	check(InQueuedWork);

	if (TimeToDie)
	{
		InQueuedWork->Abandon();
		return;
	}

	FQueuedWorkInfo WorkInfo;
	{
		VOXEL_SCOPE_COUNTER("Compute Priority");
		WorkInfo = FQueuedWorkInfo(InQueuedWork, PriorityCategory, PriorityOffset);
	}

	{
		VOXEL_SCOPE_COUNTER("Lock");
		Section.Lock();
	}
	{
		VOXEL_SCOPE_COUNTER("Add Work");
		if (Settings.bConstantPriorities)
		{
			WorkInfo.RecomputePriority(FPlatformTime::Seconds());
			StaticQueuedWorks.push(WorkInfo);
		}
		else
		{
			QueuedWorks.Add(WorkInfo);
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Wake up threads");
		for (auto* QueuedThread : QueuedThreads)
		{
			QueuedThread->DoWorkEvent->Trigger();
		}
		QueuedThreads.Reset();
	}
	
	{
		VOXEL_SCOPE_COUNTER("Unlock");
		Section.Unlock();
	}
}

void FVoxelQueuedThreadPool::AddQueuedWorks(const TArray<IVoxelQueuedWork*>& InQueuedWorks, uint32 PriorityCategory, int32 PriorityOffset)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(IsInGameThread());

	if (TimeToDie)
	{
		for (auto* InQueuedWork : InQueuedWorks)
		{
			InQueuedWork->Abandon();
		}
		return;
	}

	{
		VOXEL_SCOPE_COUNTER("Lock");
		Section.Lock();
	}

	{
		if (!Settings.bConstantPriorities)
		{
			VOXEL_SCOPE_COUNTER("Reserve");
			QueuedWorks.Reserve(QueuedWorks.Num() + InQueuedWorks.Num());
		}
		VOXEL_SCOPE_COUNTER("Add Works");
		for (auto* InQueuedWork : InQueuedWorks)
		{
			FQueuedWorkInfo WorkInfo(InQueuedWork, PriorityCategory, PriorityOffset);

			if (Settings.bConstantPriorities)
			{
				WorkInfo.RecomputePriority(FPlatformTime::Seconds());
				StaticQueuedWorks.push(WorkInfo);
			}
			else
			{
				QueuedWorks.Add(WorkInfo);
			}
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Wake up threads");
		for (auto* QueuedThread : QueuedThreads)
		{
			QueuedThread->DoWorkEvent->Trigger();
		}
		QueuedThreads.Reset();
	}

	{
		VOXEL_SCOPE_COUNTER("Unlock");
		Section.Unlock();
	}
}

IVoxelQueuedWork* FVoxelQueuedThreadPool::ReturnToPoolOrGetNextJob(FVoxelQueuedThread* InQueuedThread)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	check(InQueuedThread);

	FScopeLockWithStats Lock(Section);

	if (QueuedWorks.Num() > 0)
	{
		check(!Settings.bConstantPriorities);
		check(!TimeToDie);

		VOXEL_ASYNC_SCOPE_COUNTER("Voxel Thread Pool Recompute Priorities");

		// Find best work. We recompute every priorities as the priorities can change (eg, the camera might have moved)
		int32 BestIndex = -1;
		uint64 BestPriority = 0;
		int32 NumRecomputed = 0;
		const double Time = FPlatformTime::Seconds();
		for (int32 Index = 0; Index < QueuedWorks.Num(); Index++)
		{
			auto& WorkInfo = QueuedWorks.GetData()[Index];
			if (WorkInfo.NextPriorityUpdateTime < Time)
			{
				NumRecomputed++;
				WorkInfo.RecomputePriority(Time);
			}
			const uint64 Priority = WorkInfo.GetPriority();
			if (Priority >= BestPriority)
			{
				BestPriority = Priority;
				BestIndex = Index;
			}
		}

		INC_DWORD_STAT_BY(STAT_RecomputedVoxelTasksPriorities, NumRecomputed);

		auto* Work = QueuedWorks[BestIndex].Work;
		QueuedWorks.RemoveAtSwap(BestIndex);
		check(Work);
		return Work;
	}
	else if (!StaticQueuedWorks.empty())
	{
		check(Settings.bConstantPriorities);
		auto* Work = StaticQueuedWorks.top().Work;
		StaticQueuedWorks.pop();
		check(Work);
		return Work;
	}
	else
	{
		QueuedThreads.Add(InQueuedThread);
		return nullptr;
	}
}

void FVoxelQueuedThreadPool::AbandonAllTasks()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	ensure(!TimeToDie);
	
	{
		FScopeLockWithStats Lock(Section);
		TimeToDie = true;
		// Clean up all queued objects
		for (auto& WorkInfo : QueuedWorks)
		{
			WorkInfo.Work->Abandon();
		}
		QueuedWorks.Reset();
		while (!StaticQueuedWorks.empty())
		{
			StaticQueuedWorks.top().Work->Abandon();
			StaticQueuedWorks.pop();
		}
	}
	// Wait for all threads to finish up
	while (true)
	{
		{
			FScopeLockWithStats Lock(Section);
			if (AllThreads.Num() == QueuedThreads.Num())
			{
				break;
			}
		}
		FPlatformProcess::Sleep(0.0f);
	}
}
