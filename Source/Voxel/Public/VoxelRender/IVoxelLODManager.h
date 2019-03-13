// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

class IVoxelRenderer;
class IVoxelPool;
class FVoxelDebugManager;
class AVoxelWorldInterface;

DECLARE_DELEGATE_OneParam(FVoxelOnUpdateFinished, FIntBox);

struct FVoxelLODSettings
{
	float VoxelSize;
	int OctreeDepth;
	FIntBox WorldBounds;
	float LODUpdateRate;
	
	TWeakObjectPtr<UWorld> World;
	TSharedPtr<IVoxelRenderer> Renderer;
	TSharedPtr<IVoxelPool> Pool;
	TSharedPtr<FVoxelDebugManager, ESPMode::ThreadSafe> DebugManager;

	bool bWaitForOtherChunksToAvoidHoles;
};

class VOXEL_API IVoxelLODManager
{
public:
	const FVoxelLODSettings Settings;

	IVoxelLODManager(const FVoxelLODSettings& Settings)
		: Settings(Settings)
	{
	}
	virtual ~IVoxelLODManager() = default;

	virtual void UpdatePosition(const FIntVector& Position, bool bWaitForAllChunksToFinishUpdating = false, const FVoxelOnUpdateFinished& FinishDelegate = FVoxelOnUpdateFinished()) = 0;
	virtual void UpdateBounds(const TArray<FIntBox>& Bounds, bool bWaitForAllChunksToFinishUpdating = false, const FVoxelOnUpdateFinished& FinishDelegate = FVoxelOnUpdateFinished()) = 0;
	virtual void UpdateBounds(const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating = false, const FVoxelOnUpdateFinished& FinishDelegate = FVoxelOnUpdateFinished()) = 0;
	
	virtual void ForceLODsUpdate() = 0;
};