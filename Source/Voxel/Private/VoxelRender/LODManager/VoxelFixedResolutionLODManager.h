// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelMinimal.h"

class FVoxelFixedResolutionLODManager : public IVoxelLODManager
{
public:
	static TVoxelSharedRef<FVoxelFixedResolutionLODManager> Create(const FVoxelLODSettings& LODSettings);

	bool Initialize(int32 ChunkLOD, int32 MaxChunks);

	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }

	virtual void ForceLODsUpdate() override final {}
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const override final { return false; }

	virtual void Destroy() override final {}

private:
	using IVoxelLODManager::IVoxelLODManager;
};