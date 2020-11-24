// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelTickable.h"
#include "Containers/Ticker.h"

enum class EVoxelPlayType;
class FVoxelPool;
class FVoxelData;
class AVoxelWorld;
class UVoxelProceduralMeshComponent;

struct FVoxelDebugManagerSettings
{
	const TWeakObjectPtr<AVoxelWorld> VoxelWorld;
	const TVoxelSharedRef<FVoxelPool> Pool;
	const TVoxelSharedRef<FVoxelData> Data;
	const bool bDisabled;
	
	FVoxelDebugManagerSettings(
		const AVoxelWorld* World,
		EVoxelPlayType PlayType,
		const TVoxelSharedRef<FVoxelPool>& Pool,
		const TVoxelSharedRef<FVoxelData>& Data,
		bool bDisabled = false);
};

class VOXEL_API FVoxelGlobalDebugManager : public FTickerObjectBase
{
public:
	//~ Begin FTickerObjectBase Interface
	virtual bool Tick(float DeltaTime) override;
	//~ End FTickerObjectBase Interface
};

extern VOXEL_API FVoxelGlobalDebugManager* GVoxelDebugManager;

class VOXEL_API FVoxelDebugManager : public FVoxelTickable, public TVoxelSharedFromThis<FVoxelDebugManager>
{
public:
	const FVoxelDebugManagerSettings Settings;

	static TVoxelSharedRef<FVoxelDebugManager> Create(const FVoxelDebugManagerSettings& Settings);
	void Destroy();

public:
	void ReportUpdatedChunks(TFunction<TArray<FVoxelIntBox>()> InUpdatedChunks);
	void ReportRenderChunks(TFunction<TArray<FVoxelIntBox>()> InRenderChunks);
	void ReportMultiplayerSyncedChunks(TFunction<TArray<FVoxelIntBox>()> InMultiplayerSyncedChunks);
	
	void ReportMeshTasksCallbacksQueueNum(int32 Num);
	void ReportMeshActionQueueNum(int32 Num);

	void ReportChunkEmptyState(const FVoxelIntBox& Bounds, bool bIsEmpty);
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

	TArray<FVoxelIntBox> UpdatedChunks;
	TArray<FVoxelIntBox> RenderChunks;
	TArray<FVoxelIntBox> MultiplayerSyncedChunks;

	int32 MeshTasksCallbacksQueueNum = 0;
	int32 MeshActionQueueNum = 0;
	struct FChunkEmptyState
	{
		FVoxelIntBox Bounds;
		bool bIsEmpty;
	};
	TArray<FChunkEmptyState> ChunksEmptyStates;
};