// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"

class IVoxelRenderer;
class IVoxelPool;
class FVoxelDebugManager;
class AVoxelWorld;
class FVoxelData;

// Fired once per chunk
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdateFinished, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdate, FVoxelIntBox);

struct FVoxelLODSettings
{
	const TVoxelSharedRef<IVoxelRenderer> Renderer;
	const TVoxelSharedRef<IVoxelPool> Pool;

	const float VoxelSize;
	const int32 OctreeDepth;
	const FVoxelIntBox WorldBounds;
	const bool bConstantLOD;
	const bool bStaticWorld;
	const float MinDelayBetweenLODUpdates;
	// Update invokers positions 10 times per seconds: used for LOD updates, but also for chunk updates priority
	const float MinDelayBetweenInvokersUpdates = 0.1;
	const bool bEnableTransitions;
	const bool bInvertTransitions;

	const TWeakObjectPtr<UWorld> World;

	// If Data isn't null, it's Depth and WorldBounds will be used
	FVoxelLODSettings(
		const AVoxelWorld* World,
		EVoxelPlayType PlayType,
		const TVoxelSharedRef<IVoxelRenderer>& Renderer,
		const TVoxelSharedRef<IVoxelPool>& Pool,
		const FVoxelData* Data = nullptr);
};

class VOXEL_API IVoxelLODManager
{
public:
	FVoxelOnChunkUpdate OnChunkUpdate;
	const FVoxelLODSettings Settings;

	explicit IVoxelLODManager(const FVoxelLODSettings& Settings)
		: Settings(Settings)
	{
	}
	virtual ~IVoxelLODManager() = default;

	//~ Begin IVoxelLODManager Interface
	// Both specializations are used, and we don't want to allocate single element arrays or to do lots of virtual calls
	// Returns the number of chunks to update = number of times FinishDelegate is going to be fired
	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate = FVoxelOnChunkUpdateFinished()) = 0;
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate = FVoxelOnChunkUpdateFinished()) = 0;

	virtual void ForceLODsUpdate() = 0;
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const = 0;

	virtual void Destroy() = 0;
	//~ End IVoxelLODManager Interface
	
public:
	inline int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate)
	{
		FVoxelOnChunkUpdateFinished MulticastDelegate;
		MulticastDelegate.Add(FinishDelegate);
		return UpdateBounds(Bounds, MulticastDelegate);
	}
	inline int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate)
	{
		FVoxelOnChunkUpdateFinished MulticastDelegate;
		MulticastDelegate.Add(FinishDelegate);
		return UpdateBounds(Bounds, MulticastDelegate);
	}

	template<typename T>
	inline int32 UpdateBounds_OnAllFinished(const T& Bounds, const FSimpleDelegate& AllFinishedDelegate, const FVoxelOnChunkUpdateFinished::FDelegate& FinishDelegate = {})
	{
		TVoxelSharedRef<int32> Count = MakeVoxelShared<int32>(0);
		*Count = UpdateBounds(
			Bounds,
			FVoxelOnChunkUpdateFinished::FDelegate::CreateLambda([=](FVoxelIntBox ChunkBounds)
				{
					(*Count)--;
					ensure(*Count >= 0);
					FinishDelegate.ExecuteIfBound(ChunkBounds);
					if (*Count == 0)
					{
						AllFinishedDelegate.ExecuteIfBound();
					}
				}));
		if (*Count == 0)
		{
			// No chunk to update
			AllFinishedDelegate.ExecuteIfBound();
		}
		return *Count;
	}
};