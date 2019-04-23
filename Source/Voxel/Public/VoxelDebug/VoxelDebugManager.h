// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

class FVoxelRenderChunk;
class FVoxelData;
class AVoxelWorld;
class UVoxelProceduralMeshComponent;

class VOXEL_API FVoxelDebugManager : public TSharedFromThis<FVoxelDebugManager, ESPMode::ThreadSafe>
{
public:
	static TSharedRef<FVoxelDebugManager, ESPMode::ThreadSafe> Create()
	{
		return MakeShareable(new FVoxelDebugManager());
	}

	void Tick(float DeltaTime, AVoxelWorld* World);

	void ReportUpdatedChunks(const TArray<FIntBox>& UpdatedChunks);
	void ReportRenderChunks(const TMap<uint64, TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& RenderChunks);
	void ReportRenderOctreeBuild(const FString& Log, int32 NumberOfLeaves, bool bTooManyChunks);
	void ReportOctreeCompact(float Duration, uint32 NumDeleted);
	void ReportCacheUpdate(
		uint32 NumChunksSubdivided,
		uint32 NumChunksCached,
		uint32 NumRemovedFromCache,
		uint32 TotalNumCachedChunks,
		uint32 MaxCacheSize,
		float Duration
	);
	void ReportManualCacheProgress(int32 Current, int32 Total);
	void ReportTasksCount(int32 TaskCount);
	void ReportMultiplayerSyncedChunks(const TArray<FIntBox>& MultiplayerSyncedChunks);
	void ReportChunkEmptyState(const FIntBox& Bounds, bool bIsEmpty);
	void ClearChunksEmptyStates();
	void ReportToolFailure(const FString& ToolName, const FString& Message);

public:
	static bool ShowSpawnerRays();
	static bool ShowSpawnerHits();
	static bool ShowSpawnerPositions();

private:
	FVoxelDebugManager() = default;

	TArray<FIntBox> UpdatedChunks;
	TArray<FIntBox> RenderChunks;
	TArray<FIntBox> MultiplayerSyncedChunks;

	int32 TaskCount = 0;

	struct FChunkEmptyState
	{
		FIntBox Bounds;
		bool bIsEmpty;
	};
	TArray<FChunkEmptyState> ChunksEmptyStates;
};