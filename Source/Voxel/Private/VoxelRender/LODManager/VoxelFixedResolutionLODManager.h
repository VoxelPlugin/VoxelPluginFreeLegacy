// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "VoxelRender/IVoxelLODManager.h"

class FVoxelFixedResolutionLODManager : public IVoxelLODManager
{
public:
	FVoxelFixedResolutionLODManager(const FVoxelLODSettings& LODSettings, int ChunkLOD);

	virtual void UpdateBounds(const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}
	virtual void UpdateBounds(const TArray<FIntBox>& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}
	virtual void UpdatePosition(const FIntVector& Position, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}

	void ForceLODsUpdate() override {}
};