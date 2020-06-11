// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTickable.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelInvokerSettings.h"
#include "VoxelMinimal.h"
#include "VoxelAsyncWork.h"

class FVoxelRenderOctreeAsyncBuilder;
class FVoxelRenderOctree;
class UVoxelInvokerComponentBase;
class AVoxelWorldInterface;

struct FVoxelLODDynamicSettings
{
	int32 MinLOD;
	int32 MaxLOD;

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
	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;

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
	
	TUniquePtr<FVoxelRenderOctreeAsyncBuilder, TVoxelAsyncWorkDelete<FVoxelRenderOctreeAsyncBuilder>> Task;
	
	TVoxelSharedPtr<FVoxelRenderOctree> Octree;

	struct FVoxelInvokerInfo
	{
		FIntVector LocalPosition{ForceInit};
		FVoxelInvokerSettings Settings;
	};
	TMap<TWeakObjectPtr<UVoxelInvokerComponentBase>, FVoxelInvokerInfo> InvokerComponentsInfos;
	TArray<TWeakObjectPtr<UVoxelInvokerComponentBase>> SortedInvokerComponents;

	bool bAsyncTaskWorking = false;
	bool bLODUpdateQueued = true;
	double LastLODUpdateTime = 0;
	double LastInvokersUpdateTime = 0;

	void UpdateInvokers();
	void UpdateLODs();

	void ClearInvokerComponents();
};