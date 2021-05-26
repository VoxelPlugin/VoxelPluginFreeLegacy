// Copyright 2021 Phyronnaz

#include "VoxelThreadPool.h"
#include "VoxelQueuedWork.h"
#include "VoxelMinimal.h"

#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Misc/ScopeExit.h"
#include "Async/TaskGraphInterfaces.h"

DECLARE_DWORD_COUNTER_STAT(TEXT("Recomputed Voxel Tasks Priorities"), STAT_RecomputedVoxelTasksPriorities, STATGROUP_VoxelCounters);

FVoxelThreadPool* GVoxelThreadPool = nullptr;

TAutoConsoleVariable<float> CVarVoxelThreadingPriorityDuration(
	TEXT("voxel.threading.PriorityDuration"),
	0.5,
	TEXT("Task priorities will be recomputed with the new invoker position every PriorityDuration seconds"),
	ECVF_Default);

TAutoConsoleVariable<int32> CVarVoxelThreadingNumThreads(
	TEXT("voxel.threading.NumThreads"),
	2,
	TEXT("The number of threads to use to process voxel tasks"),
	ECVF_Default);

TAutoConsoleVariable<int32> CVarVoxelThreadingThreadPriority(
	TEXT("voxel.threading.ThreadPriority"),
	2,
	TEXT(
		"0: Normal"
		"1: AboveNormal"
		"2: BelowNormal"
		"3: Highest"
		"4: Lowest"
		"5: SlightlyBelowNormal"
		"6: TimeCritical"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static FAutoConsoleCommand CmdAbandonAllTasks(
    TEXT("voxel.threading.AbandonAllTasks"),
    TEXT("Will abandon all active tasks"),
	FConsoleCommandDelegate::CreateLambda([](){ GVoxelThreadPool->AbandonAllTasks(); }));

static FAutoConsoleCommand CmdLogThreadPoolStats(
    TEXT("voxel.threading.LogStats"),
    TEXT(""),
	FConsoleCommandDelegate::CreateLambda([](){ GVoxelThreadPool->LogTimes(); }));

static FAutoConsoleCommand CmdClearThreadPoolStats(
    TEXT("voxel.threading.ClearStats"),
    TEXT(""),
	FConsoleCommandDelegate::CreateLambda([](){ GVoxelThreadPool->ClearTimes(); }));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelThread::FVoxelThread(FVoxelThreadPool& Pool, const FString& ThreadName, uint32 StackSize, EThreadPriority ThreadPriority)
	: ThreadName(ThreadName)
	, ThreadPool(Pool)
	, Event(*FPlatformProcess::GetSynchEventFromPool())
	, TimeToDie(false)
	// NOTE: make sure to create Thread last, so that everything is setup when Run is called
	, Thread(FRunnableThread::Create(this, *ThreadName, StackSize, ThreadPriority, FPlatformAffinity::GetPoolThreadMask()))
{
	check(Thread);
}

FVoxelThread::~FVoxelThread()
{
	// Tell the thread it needs to die
	TimeToDie = true;
	// Trigger the thread so that it will come out of the wait state if
	// it isn't actively doing work
	Event.Trigger();
	// Kill (but wait for thread to finish)
	Thread->Kill(true);
	// Clean up the event
	FPlatformProcess::ReturnSynchEventToPool(&Event);
}

uint32 FVoxelThread::Run()
{
	while (!TimeToDie)
	{
		// We need to wait for shorter amount of time
		bool bContinueWaiting = true;
		while (bContinueWaiting)
		{
			VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER("WaitForWork");
			
			// Wait for some work to do
			bContinueWaiting = !Event.Wait(10);
		}

		if (TimeToDie)
		{
			break;
		}
		
		IVoxelQueuedWork* LocalQueuedWork = ThreadPool.ReturnToPoolOrGetNextJob(this);

		while (LocalQueuedWork)
		{
			const FName Name = LocalQueuedWork->Name;
			const EVoxelTaskType Type = LocalQueuedWork->TaskType;
			const FVoxelPoolId PoolId = LocalQueuedWork->PoolId;

			const double StartTime = FPlatformTime::Seconds();

			LocalQueuedWork->DoThreadedWork();
			// IMPORTANT: LocalQueuedWork should be considered as deleted after this line

			const double EndTime = FPlatformTime::Seconds();

			{
				FScopeLock Lock(&ThreadPool.CountersSection);
				ThreadPool.GlobalCounters.Decrement(Type);
				ThreadPool.PoolsCounters[PoolId].Decrement(Type);
			}

			{
				FScopeLock Lock(&ThreadPool.StatsSection);
				ThreadPool.Stats.FindOrAdd(Name) += EndTime - StartTime;
			}

			LocalQueuedWork = ThreadPool.ReturnToPoolOrGetNextJob(this);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelThreadPool::FVoxelThreadPool()
{
	TFunction<void()> ShutdownCallback = [WeakIsAlive = MakeVoxelWeakPtr(IsAlive), this]()
	{
		if (WeakIsAlive.IsValid())
		{
			AbandonAllTasks();
		}
	};
	FTaskGraphInterface::Get().AddShutdownCallback(ShutdownCallback);
}

FVoxelThreadPool::~FVoxelThreadPool()
{
	AbandonAllTasks();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelThreadPool::AbandonAllTasks()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	check(!IsAbandoningAllTasks);
	IsAbandoningAllTasks = true;
	{
		// It's safe to lock because we have IsAbandoningAllTasks set to true, so threads can't lock
		FVoxelScopeLockWithStats Lock(CriticalSection);

		// Clean up all queued objects
		for (auto& WorkInfo : QueuedWorks)
		{
			AbandonWork(*WorkInfo.Work);
		}
		QueuedWorks.Reset();
		QueuedThreads.Reset();

		// Wait for all threads to finish up
		// Safe because the thread destructor will wait on the runnable
		// Due to IsAbandoningAllTasks, they can't pick another job either
		AllThreads.Reset();
	}
	check(IsAbandoningAllTasks);
	IsAbandoningAllTasks = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TUniquePtr<FVoxelThread> FVoxelThreadPool::CreateThread()
{
#if VOXEL_ENGINE_VERSION < 426
	TRACE_THREAD_GROUP_SCOPE("VoxelThreadPool");
#else
	UE_5_ONLY(UE::)Trace::ThreadGroupBegin(TEXT("VoxelThreadPool"));
	ON_SCOPE_EXIT
	{
		UE_5_ONLY(UE::)Trace::ThreadGroupEnd();
	};
#endif

	static int32 ThreadIndex = 0;
	const FString Name = FString::Printf(TEXT("Voxel Thread %d"), ThreadIndex++);
	return MakeUnique<FVoxelThread>(*this, Name, 1024 * 1024, EThreadPriority(FMath::Clamp(CVarVoxelThreadingThreadPriority.GetValueOnGameThread(), 0, 6)));
}

void FVoxelThreadPool::AbandonWork(IVoxelQueuedWork& Work)
{
	{
		FScopeLock Lock(&CountersSection);
		GlobalCounters.Decrement(Work.TaskType);
		PoolsCounters[Work.PoolId].Decrement(Work.TaskType);
	}

	Work.Abandon();
}

IVoxelQueuedWork* FVoxelThreadPool::ReturnToPoolOrGetNextJob(FVoxelThread* InQueuedThread)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	check(InQueuedThread);

	if (IsAbandoningAllTasks)
	{
		// Just exit now, to avoid deadlock
		return nullptr;
	}
	
	const double StartTime = FPlatformTime::Seconds();
	ON_SCOPE_EXIT
	{
		const double EndTime = FPlatformTime::Seconds();
		FScopeLock Lock(&StatsSection);
		Stats.FindOrAdd(STATIC_FNAME("Find Work")) += EndTime - StartTime;
	};

	FVoxelScopeLockWithStats Lock(CriticalSection);

	const double Time = FPlatformTime::Seconds();
	if (Time > LastPriorityComputeTime + CVarVoxelThreadingPriorityDuration.GetValueOnAnyThread())
	{
		LastPriorityComputeTime = Time;
		RecomputePriorities_AssumeLocked();
	}

	if (QueuedWorks.Num() > 0)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("HeapPop");
		FQueuedWorkInfo WorkInfo;
		QueuedWorks.HeapPop(WorkInfo, false);
		WorkInfo.Work->CheckIsValidLowLevel();
		return WorkInfo.Work;
	}

	// Sleep thread
	QueuedThreads.Add(InQueuedThread);
	return nullptr;
}

void FVoxelThreadPool::RecomputePriorities_AssumeLocked()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Recompute priorities");

		for (int32 Index = 0; Index < QueuedWorks.Num(); Index++)
		{
			FQueuedWorkInfo& WorkInfo = QueuedWorks.GetData()[Index];
			WorkInfo.Work->CheckIsValidLowLevel();
			
			if (WorkInfo.Work->ShouldAbandon())
			{
				AbandonWork(*WorkInfo.Work);
				QueuedWorks.RemoveAtSwap(Index, 1, false);
				Index--;
				continue;
			}
			
			if (WorkInfo.Work->Priority != IVoxelQueuedWork::EPriority::Null)
			{
				WorkInfo.RecomputePriority();
			}
		}
	}

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Heapify");
		QueuedWorks.Heapify();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelThreadPool::LogTimes() const
{
	FScopeLock Lock(&StatsSection);
	LOG_VOXEL(Log, TEXT("#############################################"));
	LOG_VOXEL(Log, TEXT("########## Voxel Thread Pool Stats ##########"));
	LOG_VOXEL(Log, TEXT("#############################################"));
	for (const auto& It : Stats)
	{
		LOG_VOXEL(Log, TEXT("%s: %fs"), *It.Key.ToString(), It.Value);
	}
}

void FVoxelThreadPool::ClearTimes()
{
	FScopeLock Lock(&StatsSection);
	Stats.Reset();
}