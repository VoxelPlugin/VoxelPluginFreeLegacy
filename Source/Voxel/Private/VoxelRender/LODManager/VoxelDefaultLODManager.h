// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTickable.h"
#include "VoxelAsyncWork.h"
#include "VoxelInvokerSettings.h"
#include "VoxelRender/IVoxelLODManager.h"
#include "VoxelDefaultLODManager.generated.h"

class FVoxelRenderOctree;
class UVoxelInvokerComponentBase;
class FVoxelRenderOctreeAsyncBuilder;

UCLASS()
class UVoxelDefaultLODSubsystemProxy : public UVoxelLODSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelDefaultLODManager);
};

class FVoxelDefaultLODManager : public IVoxelLODManager, public FVoxelTickable
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelDefaultLODSubsystemProxy);
	
	//~ Begin IVoxelSubsystem Interface
	virtual void Create() override;
	virtual void Destroy() override;
	//~ End IVoxelSubsystem Interface
	
	//~ Begin IVoxelLODManager Interface	
	virtual int32 UpdateBounds(const FVoxelIntBox& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;
	virtual int32 UpdateBounds(const TArray<FVoxelIntBox>& Bounds, const FVoxelOnChunkUpdateFinished& FinishDelegate) override final;

	virtual void ForceLODsUpdate() override final;
	virtual bool AreCollisionsEnabled(const FIntVector& Position, uint8& OutLOD) const override final;
	//~ End IVoxelLODManager Interface

	//~ Begin FVoxelTickable Interface
	virtual void Tick(float DeltaTime) override;
	//~ End FVoxelTickable Interface

private:
	TVoxelAsyncWorkPtr<FVoxelRenderOctreeAsyncBuilder> Task;
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