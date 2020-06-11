// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformAffinity.h"
#include "HAL/ThreadSafeBool.h"
#include "VoxelMinimal.h"
#include <queue>

class IVoxelQueuedWork;
class FVoxelQueuedThread;

class VOXEL_API FVoxelQueuedThreadPoolStats
{
public:
	static FVoxelQueuedThreadPoolStats& Get();

	void Report(FName Name, double Time);
	void LogTimes() const;

private:
	FVoxelQueuedThreadPoolStats() = default;
	
	mutable FCriticalSection Section;
	TMap<FName, double> Times;
};

struct VOXEL_API FVoxelQueuedThreadPoolSettings
{
	const FString PoolName;
	const uint32 NumThreads;
	const uint32 StackSize;
	const EThreadPriority ThreadPriority;
	const bool bConstantPriorities;

	FVoxelQueuedThreadPoolSettings(
		const FString& PoolName, 
		uint32 NumThreads, 
		uint32 StackSize, 
		EThreadPriority ThreadPriority, 
		bool bConstantPriorities);
};

class VOXEL_API FVoxelQueuedThreadPool : public TVoxelSharedFromThis<FVoxelQueuedThreadPool>
{
public:
	const FVoxelQueuedThreadPoolSettings Settings;

	static TVoxelSharedRef<FVoxelQueuedThreadPool> Create(const FVoxelQueuedThreadPoolSettings& Settings);
	~FVoxelQueuedThreadPool();

	int32 GetNumPendingWorks() const
	{
		// Not really thread safe, only use this for debug
		// Also count active threads
		return (Settings.bConstantPriorities ? StaticQueuedWorks.size() : QueuedWorks.Num()) + GetNumThreads() - QueuedThreads.Num();
	}
	int32 GetNumThreads() const
	{
		return AllThreads.Num();
	}
	
	// Final priority is 64 bits: PriorityCategory in upper bits, and GetPriority in lower bits
	// Use PriorityCategory to make some type of tasks have a higher priority than other
	void AddQueuedWork(IVoxelQueuedWork* InQueuedWork, uint32 PriorityCategory, int32 PriorityOffset);
	void AddQueuedWorks(const TArray<IVoxelQueuedWork*>& InQueuedWorks, uint32 PriorityCategory, int32 PriorityOffset);

	IVoxelQueuedWork* ReturnToPoolOrGetNextJob(FVoxelQueuedThread* InQueuedThread);

	void AbandonAllTasks();

private:
	explicit FVoxelQueuedThreadPool(const FVoxelQueuedThreadPoolSettings& Settings);

	const TArray<TUniquePtr<FVoxelQueuedThread>> AllThreads;

	FCriticalSection Section;
	TArray<FVoxelQueuedThread*> QueuedThreads;

	struct FQueuedWorkInfo
	{
		IVoxelQueuedWork* Work;
		double NextPriorityUpdateTime;
		uint32 PriorityCategory;
		uint32 Priority;
		int32 PriorityOffset;

		FQueuedWorkInfo() = default;
		FQueuedWorkInfo(
			IVoxelQueuedWork* Work,
			uint32 PriorityCategory,
			int32 PriorityOffset)
			: Work(Work)
			, NextPriorityUpdateTime(0)
			, PriorityCategory(PriorityCategory)
			, Priority(0)
			, PriorityOffset(PriorityOffset)
		{
		}

		void RecomputePriority(double Time);

		FORCEINLINE uint64 GetPriority() const
		{
			return (uint64(PriorityCategory) << 32) | uint64(Priority);
		}
		FORCEINLINE bool operator<(const FQueuedWorkInfo& Other) const
		{
			return GetPriority() < Other.GetPriority();
		}
	};
	TArray<FQueuedWorkInfo> QueuedWorks;
	std::priority_queue<FQueuedWorkInfo> StaticQueuedWorks;
	
	FThreadSafeBool TimeToDie = false;
};