// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelEnums.h"
#include "VoxelQueuedWork.h"
#include "HAL/Event.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "HAL/PlatformAffinity.h"
#include "HAL/IConsoleManager.h"
#include "VoxelContainers/VoxelStaticArray.h"

class IVoxelQueuedWork;
class FVoxelThread;
class FVoxelThreadPool;

extern VOXEL_API FVoxelThreadPool* GVoxelThreadPool;

extern VOXEL_API TAutoConsoleVariable<float> CVarVoxelThreadingPriorityDuration;
extern VOXEL_API TAutoConsoleVariable<int32> CVarVoxelThreadingNumThreads;
extern VOXEL_API TAutoConsoleVariable<int32> CVarVoxelThreadingThreadPriority;

class VOXEL_API FVoxelThread : public FRunnable
{
public:
	FVoxelThread(FVoxelThreadPool& Pool, const FString& ThreadName, uint32 StackSize, EThreadPriority ThreadPriority);
	~FVoxelThread();

	//~ Begin FRunnable Interface
	virtual uint32 Run() override;
	//~ End FRunnable Interface

	void Wakeup() const
	{
		Event.Trigger();
	}

private:
	const FString ThreadName;
	FVoxelThreadPool& ThreadPool;
	FEvent& Event;
	/** If true, the thread should exit. */
	FThreadSafeBool TimeToDie;

	const TUniquePtr<FRunnableThread> Thread;
};

class VOXEL_API FVoxelThreadPool
{
public:
	FVoxelThreadPool();
	~FVoxelThreadPool();
	
	// Final priority is 64 bits: PriorityCategory in upper bits, and GetPriority in lower bits
	// Use PriorityCategory to make some type of tasks have a higher priority than other
	template<typename Array>
	void AddQueuedWorks(const Array& InQueuedWorks, uint32 PriorityCategory, int32 PriorityOffset, FVoxelPoolId PoolId, EVoxelTaskType Type)
	{
		VOXEL_FUNCTION_COUNTER();
		check(IsInGameThread());

		{
			FScopeLock Lock(&CountersSection);
			GlobalCounters.Add(Type, InQueuedWorks.Num());
			PoolsCounters.FindOrAdd(PoolId).Add(Type, InQueuedWorks.Num());
		}

		for (auto* InQueuedWork : InQueuedWorks)
		{
			ensureVoxelSlow(InQueuedWork->TaskType == Type);
			InQueuedWork->PoolId = PoolId;
		}

		FVoxelScopeLockWithStats Lock(CriticalSection);

		{
			VOXEL_SCOPE_COUNTER("Add Works");
			for (auto* InQueuedWork : InQueuedWorks)
			{
				FQueuedWorkInfo WorkInfo(InQueuedWork, PriorityCategory, PriorityOffset);
				WorkInfo.RecomputePriority();
				QueuedWorks.HeapPush(WorkInfo);
			}
		}

		VOXEL_SCOPE_COUNTER("Wakeup threads");
		const int32 WantedActiveThreads = CVarVoxelThreadingNumThreads.GetValueOnGameThread();
		while (AllThreads.Num() - QueuedThreads.Num() < WantedActiveThreads)
		{
			if (QueuedThreads.Num() > 0)
			{
				auto* Thread = QueuedThreads.Pop(false);
				Thread->Wakeup();
			}
			else
			{
				auto& Thread = AllThreads.Emplace_GetRef(CreateThread());
				Thread->Wakeup();
			}
		}
	}

	void AbandonAllTasks();
	
private:
	TUniquePtr<FVoxelThread> CreateThread();
	void AbandonWork(IVoxelQueuedWork& Work);
	IVoxelQueuedWork* ReturnToPoolOrGetNextJob(FVoxelThread* InQueuedThread);
	void RecomputePriorities_AssumeLocked();

	friend class FVoxelThread;

	struct FQueuedWorkInfo
	{
		uint64 Priority;
		IVoxelQueuedWork* Work;

		FQueuedWorkInfo() = default;
		FQueuedWorkInfo(
			IVoxelQueuedWork* Work,
			uint32 PriorityCategory,
			int32 PriorityOffset)
			: Priority(uint64(PriorityCategory) << 32)
			, Work(Work)
		{
			// Store rarely accessed data on the Work, to save up cache
			Work->PriorityOffset = PriorityOffset;
		}

		FORCEINLINE void RecomputePriority()
		{
			const uint32 PriorityLow = FMath::Clamp<int64>(int64(Work->GetPriority()) + Work->PriorityOffset, MIN_uint32, MAX_uint32);
			const uint32 PriorityHigh = Priority >> 32;

			Priority = (uint64(PriorityHigh) << 32) | PriorityLow;
		}

		FORCEINLINE uint64 GetPriority() const
		{
			return Priority;
		}
		FORCEINLINE bool operator<(const FQueuedWorkInfo& Other) const
		{
			return GetPriority() > Other.GetPriority();
		}
	};
private:
	const TVoxelSharedRef<const uint32> IsAlive = MakeVoxelShared<uint32>();
	
	FCriticalSection CriticalSection;
	// All the threads
	TArray<TUniquePtr<FVoxelThread>> AllThreads;
	// Sleeping threads
	TArray<FVoxelThread*> QueuedThreads;
	// Heapified
	TArray<FQueuedWorkInfo> QueuedWorks;

	// Last time we computed priorities
	double LastPriorityComputeTime = 0;
	// Used to avoid deadlock with threads querying jobs
	FThreadSafeBool IsAbandoningAllTasks = false;
	
private:
	mutable FCriticalSection StatsSection;
	TMap<FName, double> Stats;

public:
	void LogTimes() const;
	void ClearTimes();

public:
	class FTaskCounters
	{
	public:
		int32 GetTotalNumTasks() const
		{
			return Total;
		}
		int32 GetNumTasksForType(EVoxelTaskType Type) const
		{
			return PerType[int32(Type)];
		}

		void Add(EVoxelTaskType Type, int32 Count)
		{
			Total += Count;
			PerType[int32(Type)] += Count;
		}
		void Decrement(EVoxelTaskType Type)
		{
			ensure(Total-- >= 0);
			ensure(PerType[int32(Type)]-- >= 0);
		}

	private:
		int32 Total = 0;
		TVoxelStaticArray<int32, int32(EVoxelTaskType::Max)> PerType{ ForceInit };
	};

	FTaskCounters GetGlobalCounters() const
	{
		FScopeLock Lock(&CountersSection);
		return GlobalCounters;
	}
	FTaskCounters GetCountersForPool(FVoxelPoolId PoolId) const
	{
		FScopeLock Lock(&CountersSection);
		return PoolsCounters.FindRef(PoolId);
	}

private:
	mutable FCriticalSection CountersSection;
	FTaskCounters GlobalCounters;
	TMap<FVoxelPoolId, FTaskCounters> PoolsCounters;
};