// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelOctree.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelDefaultLODManager.h"

#include "HAL/ThreadSafeBool.h"
#include "Async/AsyncWork.h"

class FVoxelRenderOctree;
struct FVoxelLODSettings;
class FVoxelDebugManager;

DECLARE_DELEGATE(FOnVoxelAsyncOctreeBuilderFinished);

struct FVoxelRenderOctreeSettings
{
	uint8 LODLimit;
	FIntBox WorldBounds;

	TArray<FVoxelInvoker> Invokers;

	// If SquaredDistance < SquaredLODsDistances[LOD], subdivide
	TArray<uint64> SquaredLODsDistances;

	int32 ChunksCullingLOD;

	bool bEnableRender;

	bool bEnableCollisions;
	bool bComputeVisibleChunksCollisions;
	int32 VisibleChunksCollisionsMaxLOD;

	bool bEnableNavmesh;

	bool bEnableTessellation;
	uint64 SquaredTessellationDistance;
};

class FVoxelRenderOctreeAsyncBuilder : public IQueuedWork
{
public:
	TArray<FVoxelChunkToAdd> ChunksToAdd;
	TArray<FVoxelChunkToUpdate> ChunksToUpdate;
	TArray<FVoxelChunkToRemove> ChunksToRemove;
	TArray<FVoxelTransitionsToUpdate> TransitionsToUpdate;
	TArray<FIntBox> ChunksWithLOD0Collisions;

	TSharedPtr<FVoxelRenderOctree, ESPMode::ThreadSafe> NewOctree;
	TSharedPtr<FVoxelRenderOctree, ESPMode::ThreadSafe> OldOctree;

	FOnVoxelAsyncOctreeBuilderFinished Delegate;

	FVoxelRenderOctreeAsyncBuilder(uint8 OctreeDepth, const FIntBox& WorldBounds);

public:
	void Init(const FVoxelRenderOctreeSettings& OctreeSettings, TSharedPtr<FVoxelRenderOctree, ESPMode::ThreadSafe> Octree);
	void ReportBuildTime(FVoxelDebugManager& DebugManager);

private:
	void DoWork();

public:
	//~ Begin IQueuedWork Interface
	void DoThreadedWork() override;
	void Abandon() override;
	//~ End IQueuedWork Interface

	bool IsDone() const { return bIsDone; }
	void Autodelete();

private:
	FCriticalSection DoneSection;
	FThreadSafeBool bIsDone = true;
	FThreadSafeBool bAutodelete = false;

private:
	const uint8 OctreeDepth;
	const FIntBox WorldBounds;

	FVoxelRenderOctreeSettings OctreeSettings;

	bool bTooManyChunks = false;
	double Counter;
	FString Log;
	int32 NumberOfChunks = 0;
};

class FVoxelRenderOctree : public TVoxelOctree<FVoxelRenderOctree, CHUNK_SIZE>
{
public:
	const uint64 ChunkId;
	FVoxelRenderOctree* const Root;

	enum class EDivisionType : uint8
	{
		Uninitialized = 0,
		ByDistance    = 1,
		ByNeighbors   = 2,
		ByOthers      = 3
	};

	struct FChunkSettings
	{
		FVoxelRenderChunkSettings Settings{false, false, false, false, false};
		EDivisionType DivisionType = EDivisionType::Uninitialized;
		EDivisionType OldDivisionType = EDivisionType::Uninitialized;
		uint8 TransitionMask = 0;
	}; 
	FChunkSettings ChunkSettings;
	int32 CurrentChunksCount = 0;

	inline const FVoxelRenderChunkSettings& GetSettings() const { return ChunkSettings.Settings; }

	FVoxelRenderOctree(uint8 LOD);
	FVoxelRenderOctree(const FVoxelRenderOctree* Source);

	FVoxelRenderOctree(FVoxelRenderOctree* Parent, uint8 ChildIndex);
	FVoxelRenderOctree(FVoxelRenderOctree* Parent, uint8 ChildIndex, const FChildrenArray& SourceChildren);

	~FVoxelRenderOctree();

	void ResetDivisionType();
	bool UpdateSubdividedByDistance(const FVoxelRenderOctreeSettings& Settings);
	bool UpdateSubdividedByNeighbors(const FVoxelRenderOctreeSettings& Settings);
	void ReuseOldNeighbors();
	void UpdateSubdividedByOthers(const FVoxelRenderOctreeSettings& Settings);
	void DeleteChunks(TArray<FVoxelChunkToRemove>& ChunksToRemove);

	void GetUpdates(
		bool bRecomputeTransitionMasks,
		const FVoxelRenderOctreeSettings& Settings, 
		TArray<FVoxelChunkToAdd>& ChunksToAdd, 
		TArray<FVoxelChunkToUpdate>& ChunksToUpdate,
		TArray<FVoxelChunkToRemove>& ChunksToRemove,
		TArray<FVoxelTransitionsToUpdate>& TransitionsToUpdate,
		TArray<FIntBox>& ChunksWithLOD0Collisions,
		bool bVisible = true);

	void GetChunksToUpdateForBounds(const FIntBox& Bounds, TArray<uint64>& ChunksToUpdate) const;
	void GetVisibleChunksOverlappingBounds(const FIntBox& Bounds, TArray<uint64>& VisibleChunks) const;

	bool IsCanceled() const;

private:
	bool ShouldSubdivideByDistance(const FVoxelRenderOctreeSettings& Settings) const;
	bool ShouldSubdivideByNeighbors(const FVoxelRenderOctreeSettings& Settings) const;
	bool ShouldSubdivideByOthers(const FVoxelRenderOctreeSettings& Settings) const;
	
	const FVoxelRenderOctree* GetVisibleAdjacentChunk(EVoxelDirection Direction, int32 Index) const;
	
	uint64 GetId();
};