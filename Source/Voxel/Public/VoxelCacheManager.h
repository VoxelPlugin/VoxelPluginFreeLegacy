// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelThreadPool.h"
#include "Math/NumericLimits.h"

class FVoxelDataOctree;
class AVoxelWorld;
class FVoxelData;

class FVoxelAsyncCacheWork : public FVoxelAsyncWork
{
public:
	// Output
	TMap<FIntBox, FVoxelDataOctree*> CachedOctrees;

	FVoxelAsyncCacheWork(TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data, const TArray<FIntBox>& BoundsToCache)
		: Data(Data)
		, BoundsToCache(BoundsToCache)
	{
	}

	virtual void DoWork() override;

	virtual uint64 GetPriority() const override
	{
		return MAX_uint64;
	}

private:
	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> const Data;
	TArray<FIntBox> const BoundsToCache;
};

// Autodelete
class FVoxelAsyncClearCacheWork : public FVoxelAsyncWork
{
public:
	FVoxelAsyncClearCacheWork(TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data, const TArray<FVoxelDataOctree*>& OctreesToClear)
		: FVoxelAsyncWork(nullptr, true)
		, Data(Data)
		, OctreesToClear(OctreesToClear)
	{
	}

	virtual void DoWork() override;

	virtual uint64 GetPriority() const override
	{
		return 0;
	}

private:
	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> const Data;
	TArray<FVoxelDataOctree*> const OctreesToClear;
};

class FVoxelCacheManager
{
public:
	FVoxelCacheManager(AVoxelWorld* World)
		: World(World)
	{
	}
	~FVoxelCacheManager();

	void Cache(const TArray<FIntBox>& BoundsToCache);
	void ClearCache(const TArray<FIntBox>& BoundsToKeepCached);

	void DebugCache(bool bHideEmpty, float DeltaTime, float Thickness);

	inline bool IsCaching() { UpdateFromWork(); return CacheWork.IsValid(); }

	static const FColor CreatedDirty;
	static const FColor CreatedCached;
	static const FColor CreatedNotCached;
	static const FColor Empty;

private:
	AVoxelWorld* const World;
	TMap<FIntBox, FVoxelDataOctree*> CachedOctrees;
	TUniquePtr<FVoxelAsyncCacheWork> CacheWork;

	void UpdateFromWork();
};