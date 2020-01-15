// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelTickable.h"

enum class EVoxelPlayType;
class IVoxelPool;
class FVoxelData;
class AVoxelWorld;
class UVoxelProceduralMeshComponent;

struct FVoxelDebugManagerSettings
{
	const TWeakObjectPtr<AVoxelWorld> VoxelWorld;
	const TVoxelSharedRef<IVoxelPool> Pool;
	const TVoxelSharedRef<FVoxelData> Data;
	const bool bDisabled;
	
	FVoxelDebugManagerSettings(
		const AVoxelWorld* World,
		EVoxelPlayType PlayType,
		const TVoxelSharedRef<IVoxelPool>& Pool,
		const TVoxelSharedRef<FVoxelData>& Data,
		bool bDisabled = false);
};

class VOXEL_API FVoxelDebugManager : public FVoxelTickable, public TVoxelSharedFromThis<FVoxelDebugManager>
{
public:
	const FVoxelDebugManagerSettings Settings;

	static TVoxelSharedRef<FVoxelDebugManager> Create(const FVoxelDebugManagerSettings& Settings);
	void Destroy();

public:
	void ReportUpdatedChunks(TFunction<TArray<FIntBox>()> InUpdatedChunks);
	void ReportRenderChunks(TFunction<TArray<FIntBox>()> InRenderChunks);
	void ReportMultiplayerSyncedChunks(TFunction<TArray<FIntBox>()> InMultiplayerSyncedChunks);

	void ReportMeshTaskCount(int32 TaskCount);
	void ReportMeshTasksCallbacksQueueNum(int32 Num);
	void ReportMeshActionQueueNum(int32 Num);
	void ReportFoliageTaskCount(int32 TaskCount);
	
	void ReportChunkEmptyState(const FIntBox& Bounds, bool bIsEmpty);
	void ClearChunksEmptyStates();

public:
	static bool ShowCollisionAndNavmeshDebug();
	static FColor GetCollisionAndNavmeshDebugColor(bool bEnableCollisions, bool bEnableNavmesh);

protected:
	//~ Begin FVoxelTickable Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	//~ End FVoxelTickable Interface

private:
	explicit FVoxelDebugManager(const FVoxelDebugManagerSettings& Settings);

	TArray<FIntBox> UpdatedChunks;
	TArray<FIntBox> RenderChunks;
	TArray<FIntBox> MultiplayerSyncedChunks;

	int32 MeshTaskCount = 0;
	int32 MeshTasksCallbacksQueueNum = 0;
	int32 MeshActionQueueNum = 0;
	FThreadSafeCounter FoliageTaskCount;

	struct FChunkEmptyState
	{
		FIntBox Bounds;
		bool bIsEmpty;
	};
	TArray<FChunkEmptyState> ChunksEmptyStates;
};