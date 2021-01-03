// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTickable.h"
#include "Containers/Ticker.h"
#include "VoxelSubsystem.h"
#include "VoxelDebugManager.generated.h"

class VOXEL_API FVoxelGlobalDebugManager : public FTickerObjectBase
{
public:
	//~ Begin FTickerObjectBase Interface
	virtual bool Tick(float DeltaTime) override;
	//~ End FTickerObjectBase Interface
};

extern VOXEL_API FVoxelGlobalDebugManager* GVoxelDebugManager;

UCLASS()
class VOXEL_API UVoxelDebugSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelDebugManager);
};

class VOXEL_API FVoxelDebugManager : public IVoxelSubsystem, public FVoxelTickable
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelDebugSubsystemProxy);
	
	//~ Begin IVoxelSubsystem Interface
	virtual void Destroy() override;
	//~ End IVoxelSubsystem Interface

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
	//~ End FVoxelTickable Interface

private:
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