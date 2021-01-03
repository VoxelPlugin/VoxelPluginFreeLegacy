// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelFixedResolutionLODManager.generated.h"

UCLASS()
class UVoxelFixedResolutionLODSubsystemProxy : public UVoxelLODSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelFixedResolutionLODManager);
};

class FVoxelFixedResolutionLODManager : public IVoxelLODManager
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelFixedResolutionLODSubsystemProxy);

	bool Initialize(
		int32 ChunkLOD,
		int32 MaxChunks,
		bool bVisible,
		bool bEnableCollisions,
		bool bEnableNavmesh);

	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final { return 0; }

	virtual void ForceLODsUpdate() override final {}
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const override final { return false; }
};