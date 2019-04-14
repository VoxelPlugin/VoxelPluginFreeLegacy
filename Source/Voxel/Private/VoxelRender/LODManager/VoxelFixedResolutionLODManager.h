// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "VoxelRender/IVoxelLODManager.h"

class FVoxelFixedResolutionLODManager : public IVoxelLODManager
{
public:
	static TSharedRef<FVoxelFixedResolutionLODManager> Create(const FVoxelLODSettings& LODSettings, int32 ChunkLOD);

	virtual void UpdateBounds(const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}
	virtual void UpdateBounds(const TArray<FIntBox>& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}
	virtual void UpdatePosition(const FIntVector& Position, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final {}

	virtual void ForceLODsUpdate() final {}
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const final { return false; }

private:
	FVoxelFixedResolutionLODManager(const FVoxelLODSettings& LODSettings)
		: IVoxelLODManager(LODSettings)
	{
	}
};