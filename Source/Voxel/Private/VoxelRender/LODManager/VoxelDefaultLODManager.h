// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTickable.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelGlobals.h"

class FVoxelRenderOctreeAsyncBuilder;
class FVoxelRenderOctree;
class UVoxelInvokerComponent;
class AVoxelWorldInterface;

struct FVoxelLODDynamicSettings
{
	int32 MinLOD;
	int32 MaxLOD;
	TMap<uint8, float> LODToMinDistance;

	// In world space
	float InvokerDistanceThreshold;
	
	int32 ChunksCullingLOD;

	bool bEnableRender;

	bool bEnableCollisions;
	bool bComputeVisibleChunksCollisions;
	int32 VisibleChunksCollisionsMaxLOD;
	
	bool bEnableNavmesh;
	bool bComputeVisibleChunksNavmesh;
	int32 VisibleChunksNavmeshMaxLOD;

	bool bEnableTessellation;
	float TessellationDistance; // In cm
};

class FVoxelDefaultLODManager : public IVoxelLODManager, public FVoxelTickable, public TVoxelSharedFromThis<FVoxelDefaultLODManager>
{
public:
	static TVoxelSharedRef<FVoxelDefaultLODManager> Create(
		const FVoxelLODSettings& LODSettings,
		TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
		const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings);
	~FVoxelDefaultLODManager();

	//~ Begin IVoxelLODManager Interface
	virtual int32 UpdateBounds(const FIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;
	virtual int32 UpdateBounds(const TArray<FIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;

	virtual void ForceLODsUpdate() override final;
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const override final;

	virtual void Destroy() override final;
	//~ End IVoxelLODManager Interface

	//~ Begin FVoxelTickable Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	//~ End FVoxelTickable Interface

private:
	FVoxelDefaultLODManager(
		const FVoxelLODSettings& LODSettings,
		TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
		const TVoxelSharedRef<FVoxelLODDynamicSettings>& DynamicSettings);

	const TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface;
	const TVoxelSharedRef<FVoxelLODDynamicSettings> DynamicSettings;
	
	TUniquePtr<FVoxelRenderOctreeAsyncBuilder> Task;
	
	TVoxelSharedPtr<FVoxelRenderOctree> Octree;

	TMap<TWeakObjectPtr<UVoxelInvokerComponent>, FIntVector> InvokerComponentsLocalPositions;
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> InvokerComponents;

	bool bAsyncTaskWorking = false;
	bool bLODUpdateQueued = true;
	double LastLODUpdateTime = 0;
	double LastInvokersUpdateTime = 0;

	void UpdateInvokers();
	void UpdateLODs();
	uint64 GetSquaredDistance(float DistanceInCm) const;
};