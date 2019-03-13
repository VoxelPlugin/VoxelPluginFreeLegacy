// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "VoxelRender/IVoxelLODManager.h"

class FVoxelRenderOctreeAsyncBuilder;
class FVoxelRenderOctree;
class UVoxelInvokerComponent;

struct FVoxelLODDynamicSettings
{
	int LODLimit;
	TMap<uint8, float> LODToMinDistance;
	
	float InvokerDistanceThreshold;
	
	int ChunksCullingLOD;

	bool bEnableRender;

	bool bEnableCollisions;
	bool bComputeVisibleChunksCollisions;
	int VisibleChunksCollisionsMaxLOD;
	
	bool bEnableNavmesh;

	bool bEnableGrass;
	float GrassDistance; // In cm

	bool bEnableTessellation;
	float TessellationDistance; // In cm
};

struct FVoxelInvoker
{
	FIntVector Position; 
	bool bUseForLODs;

	bool bUseForCollisions;
	uint64 SquaredCollisionsRange;
	
	bool bUseForNavmesh;
	uint64 SquaredNavmeshRange;
};

class FVoxelDefaultLODManager : public IVoxelLODManager, public TSharedFromThis<FVoxelDefaultLODManager>
{
public:
	FVoxelDefaultLODManager(
		const FVoxelLODSettings& LODSettings,
		TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface,
		const TSharedPtr<FVoxelLODDynamicSettings>& DynamicSettings);
	~FVoxelDefaultLODManager();

	virtual void UpdateBounds(const FIntBox& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final;
	virtual void UpdateBounds(const TArray<FIntBox>& Bounds, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final;
	virtual void UpdatePosition(const FIntVector& Position, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) final;

	void ForceLODsUpdate() override;

private:
	const TWeakObjectPtr<const AVoxelWorldInterface> VoxelWorldInterface;
	const TSharedPtr<FVoxelLODDynamicSettings> DynamicSettings;

	TUniquePtr<FVoxelRenderOctreeAsyncBuilder> Task;
	TSharedPtr<FVoxelRenderOctree, ESPMode::ThreadSafe> Octree;

	FTimerHandle UpdateHandle;

	TMap<TWeakObjectPtr<UVoxelInvokerComponent>, FVector> InvokersPreviousPositions;
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> PreviousInvokers;

	TArray<FVoxelInvoker> Invokers;
	TArray<uint64> SquaredLODsDistances;

	void UpdateLODs();
	void OnTaskFinished();
	void StartTimer();
	bool UpdateInvokers();
	uint64 GetSquaredDistance(float DistanceInCm) const;
};
