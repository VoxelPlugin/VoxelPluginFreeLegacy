// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

class FVoxelRenderChunk;
class FVoxelData;
class AVoxelWorld;
class UVoxelProceduralMeshComponent;

class FVoxelDebugManager
{
public:
	FVoxelDebugManager() = default;

	void Tick(float DeltaTime, AVoxelWorld* World);

	void ReportUpdatedChunks(const TArray<FIntBox>& UpdatedChunks);
	void ReportRenderChunks(const TMap<uint64, TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& RenderChunks);
	void ReportRenderOctreeBuild(const FString& Log, int NumberOfLeaves, bool bTooManyChunks);
	void ReportOctreeCompact(float Duration, uint32 NumDeleted);
	void ReportCacheUpdate(
		uint32 NumChunksSubdivided,
		uint32 NumChunksCached,
		uint32 NumRemovedFromCache,
		uint32 TotalNumCachedChunks,
		uint32 MaxCacheSize,
		float Duration
	);
	void ReportManualCacheProgress(int Current, int Total);
	void ReportTasksCount(int TaskCount);
	void ReportMultiplayerSyncedChunks(const TArray<FIntBox>& MultiplayerSyncedChunks);
	void ReportChunkEmptyState(const FIntBox& Bounds, bool bIsEmpty);
	void ClearChunksEmptyStates();
	void ReportToolFailure(const FString& ToolName, const FString& Message);

private:
	TArray<FIntBox> UpdatedChunks;
	TArray<FIntBox> RenderChunks;
	TArray<FIntBox> MultiplayerSyncedChunks;

	int TaskCount = 0;

	struct FChunkEmptyState
	{
		FIntBox Bounds;
		bool bIsEmpty;
	};
	TArray<FChunkEmptyState> ChunksEmptyStates;
};