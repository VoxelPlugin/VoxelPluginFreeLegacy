// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "IntBox.h"

class FVoxelAsyncCacheWork;
class IVoxelPool;
class FVoxelData;
class FVoxelDebugManager;

struct FVoxelCacheSettings
{
	float DataOctreeCompactDelayInSeconds;
	bool bEnableAutomaticCache;
	float CacheUpdateDelayInSeconds;
	int CacheAccessThreshold;
	int MaxCacheSize;

	TSharedPtr<FVoxelData, ESPMode::ThreadSafe> Data;
	TSharedPtr<FVoxelDebugManager, ESPMode::ThreadSafe> DebugManager;
	TSharedPtr<IVoxelPool> Pool;
	TWeakObjectPtr<UWorld> World;
};

class VOXEL_API FVoxelCacheManager : public TSharedFromThis<FVoxelCacheManager>
{
public:
	const FVoxelCacheSettings Settings;

	FVoxelCacheManager(const FVoxelCacheSettings& Settings);
	~FVoxelCacheManager();

	void Cache(const TArray<FIntBox>& BoundsToCache, bool bCacheValues, bool bCacheMaterials);
	// Will only clear manually cached chunks
	void ClearCache(const TArray<FIntBox>& BoundsToKeepCached);

private:
	TUniquePtr<FVoxelAsyncCacheWork> CacheWork;

	void CompactOctree();
	void UpdateCache();
	void UpdateManualCacheProgress();

	FTimerHandle OctreeCompactHandle;
	FTimerHandle CacheHandle;
	FTimerHandle ProgressHandle;
};