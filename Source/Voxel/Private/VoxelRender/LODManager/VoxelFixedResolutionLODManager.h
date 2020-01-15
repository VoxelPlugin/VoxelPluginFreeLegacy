// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelGlobals.h"

class FVoxelFixedResolutionLODManager : public IVoxelLODManager
{
public:
	static TVoxelSharedRef<FVoxelFixedResolutionLODManager> Create(
		const FVoxelLODSettings& LODSettings,
		int32 ChunkLOD);

	virtual int32 UpdateBounds(const FIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }
	virtual int32 UpdateBounds(const TArray<FIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }

	virtual void ForceLODsUpdate() override final {}
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const override final { return false; }

	virtual void Destroy() override final {}

private:
	using IVoxelLODManager::IVoxelLODManager;
};