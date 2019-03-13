// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelCacheManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelGlobals.h"
#include "VoxelThreadPool.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Async/Async.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "IVoxelPool.h"

class FVoxelAsyncCacheWork : public FVoxelAsyncWork
{
public:
	FThreadSafeCounter Progress;
	const int32 Total;

	FVoxelAsyncCacheWork(TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, const TArray<FIntBox>& BoundsToCache, bool bCacheValues, bool bCacheMaterials)
		: FVoxelAsyncWork("CacheWork", MAX_uint64)
		, Total(BoundsToCache.Num())
		, Data(Data)
		, BoundsToCache(BoundsToCache)
		, bCacheValues(bCacheValues)
		, bCacheMaterials(bCacheMaterials)
	{
	}

	virtual void DoWork() override
	{
		Progress.Reset();

		for (auto& Bounds : BoundsToCache)
		{
			{
				FVoxelReadWriteScopeLock Lock(Data, Bounds, "AsyncCacheWork");
				Data->CacheBounds(Bounds, true, bCacheValues, bCacheMaterials);
			}
			Progress.Increment();
			if (IsCanceled())
			{
				break;
			}
		}
	}

private:
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> const Data;
	TArray<FIntBox> const BoundsToCache;
	const bool bCacheValues;
	const bool bCacheMaterials;
};

// Autodelete
class FVoxelAsyncClearCacheWork : public FVoxelAsyncWork
{
public:
	FVoxelAsyncClearCacheWork(TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, const TArray<FIntBox>& BoundsToKeepCached)
		: FVoxelAsyncWork("ClearCacheWork", 0, FVoxelAsyncWorkCallback(), true)
		, Data(Data)
		, BoundsToKeepCached(BoundsToKeepCached)
	{
	}

	virtual void DoWork() override
	{
		FVoxelReadWriteScopeLock Lock(Data, FIntBox::Infinite, "AsyncClearCacheWork");
		Data->GetOctree()->ApplyLambda([&](auto* Octree)
		{
			if (Octree->LOD == 0 && Octree->IsManuallyCached())
			{
				for (auto& Bounds : BoundsToKeepCached)
				{
					if (Octree->OctreeBounds.Intersect(Bounds))
					{
						return;
					}
				}
				Octree->ClearManualCache();
			}
		});
	}

private:
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> const Data;
	TArray<FIntBox> const BoundsToKeepCached;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCacheManager::FVoxelCacheManager(const FVoxelCacheSettings& Settings)
	: Settings(Settings)
{
	if (Settings.bEnableAutomaticCache)
	{
		auto& TimerManager = Settings.World->GetTimerManager();
		TimerManager.SetTimer(OctreeCompactHandle, FTimerDelegate::CreateRaw(this, &FVoxelCacheManager::CompactOctree), Settings.DataOctreeCompactDelayInSeconds, true);
		TimerManager.SetTimer(CacheHandle, FTimerDelegate::CreateRaw(this, &FVoxelCacheManager::UpdateCache), Settings.CacheUpdateDelayInSeconds, true);
	}
}

FVoxelCacheManager::~FVoxelCacheManager()
{
	if (CacheWork.IsValid())
	{
		if (!CacheWork->IsDone())
		{
			CacheWork->CancelAndAutodelete();
			CacheWork.Release();
		}
	}
	if (Settings.World.IsValid())
	{
		auto& TimerManager = Settings.World->GetTimerManager();
		TimerManager.ClearTimer(OctreeCompactHandle);
		TimerManager.ClearTimer(CacheHandle);
		TimerManager.ClearTimer(ProgressHandle);
	}
}

void FVoxelCacheManager::Cache(const TArray<FIntBox>& BoundsToCache, bool bCacheValues, bool bCacheMaterials)
{
	if (CacheWork.IsValid() && CacheWork->IsDone())
	{
		CacheWork.Reset();
		auto& TimerManager = Settings.World->GetTimerManager();
		TimerManager.ClearTimer(ProgressHandle);
	}

	if (CacheWork.IsValid())
	{
		UE_LOG(LogVoxel, Warning, TEXT("Voxel Cache: Cache called but work is still pending, canceling"));
		return;
	}

	CacheWork = MakeUnique<FVoxelAsyncCacheWork>(Settings.Data.ToSharedRef(), BoundsToCache, bCacheValues, bCacheMaterials);
	Settings.Pool->QueueCacheTask(CacheWork.Get());
	
	auto& TimerManager = Settings.World->GetTimerManager();
	TimerManager.SetTimer(OctreeCompactHandle, FTimerDelegate::CreateSP(this, &FVoxelCacheManager::UpdateManualCacheProgress), 0, true);
}

void FVoxelCacheManager::ClearCache(const TArray<FIntBox>& BoundsToKeepCached)
{
	// Will autodelete
	auto* ClearCacheWork = new FVoxelAsyncClearCacheWork(Settings.Data.ToSharedRef(), BoundsToKeepCached);
	Settings.Pool->QueueCacheTask(ClearCacheWork);
}

void FVoxelCacheManager::CompactOctree()
{
	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data = Settings.Data;
	TSharedPtr<FVoxelDebugManager, ESPMode::ThreadSafe> DebugManager = Settings.DebugManager;
	AsyncTask(ENamedThreads::AnyThread, [Data, DebugManager]()
	{
		uint32 NumDeleted = 0;
		double Before = FPlatformTime::Seconds();
		Data->Compact(NumDeleted);
		double After = FPlatformTime::Seconds();
					
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			DebugManager->ReportOctreeCompact(After - Before, NumDeleted);
		});
	});
}

void FVoxelCacheManager::UpdateCache()
{
	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data = Settings.Data;
	TSharedPtr<FVoxelDebugManager, ESPMode::ThreadSafe> DebugManager = Settings.DebugManager;
	uint32 CacheAccessThreshold = Settings.CacheAccessThreshold;
	uint32 MaxCacheSize = Settings.MaxCacheSize;

	AsyncTask(ENamedThreads::AnyThread, [Data, DebugManager, CacheAccessThreshold, MaxCacheSize]()
	{
		uint32 NumChunksSubdivided = 0;
		uint32 NumChunksCached = 0;
		uint32 NumRemovedFromCache = 0;
		uint32 TotalNumCachedChunks = 0;

		double Before = FPlatformTime::Seconds();
		Data->CacheMostUsedChunks(CacheAccessThreshold, MaxCacheSize, NumChunksSubdivided, NumChunksCached, NumRemovedFromCache, TotalNumCachedChunks);
		double After = FPlatformTime::Seconds();
					
		AsyncTask(ENamedThreads::GameThread, [=]()
		{
			DebugManager->ReportCacheUpdate(NumChunksSubdivided, NumChunksCached, NumRemovedFromCache, TotalNumCachedChunks, MaxCacheSize, After - Before);
		});
	});
}

void FVoxelCacheManager::UpdateManualCacheProgress()
{
	check(CacheWork.IsValid());
	Settings.DebugManager->ReportManualCacheProgress(CacheWork->Progress.GetValue(), CacheWork->Total);
}
