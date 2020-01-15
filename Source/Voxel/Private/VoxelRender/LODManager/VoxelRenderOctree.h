// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "VoxelSimpleOctree.h"
#include "VoxelAsyncWork.h"
#include "VoxelRender/VoxelChunkToUpdate.h"

#include "HAL/ThreadSafeBool.h"

class FVoxelRenderOctree;
struct FVoxelLODSettings;
class FVoxelDebugManager;

DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdate, FIntBox);

struct FVoxelInvoker
{
	FIntVector Position; 
	bool bUseForLODs;

	bool bUseForCollisions;
	uint64 SquaredCollisionsRange;
	
	bool bUseForNavmesh;
	uint64 SquaredNavmeshRange;
};

struct FVoxelRenderOctreeSettings
{
	int32 MinLOD;
	int32 MaxLOD;
	FIntBox WorldBounds;

	TArray<FVoxelInvoker> Invokers;

	// If SquaredDistance < SquaredLODsDistances[LOD], subdivide
	TArray<uint64> SquaredLODsDistances;

	int32 ChunksCullingLOD;

	bool bEnableRender;
	bool bEnableTransitions;
	bool bInvertTransitions;

	bool bEnableCollisions;
	bool bComputeVisibleChunksCollisions;
	int32 VisibleChunksCollisionsMaxLOD;

	bool bEnableNavmesh;
	bool bComputeVisibleChunksNavmesh;
	int32 VisibleChunksNavmeshMaxLOD;

	bool bEnableTessellation;
	uint64 SquaredTessellationDistance;
};

class FVoxelRenderOctreeAsyncBuilder : public FVoxelAsyncWork
{
public:
	TArray<FVoxelChunkUpdate> ChunkUpdates;

	TVoxelSharedPtr<FVoxelRenderOctree> NewOctree;
	TVoxelSharedPtr<FVoxelRenderOctree> OldOctree;

	// We don't want to do the deletion on the game thread
	TVoxelSharedPtr<FVoxelRenderOctree> OctreeToDelete;

	FVoxelRenderOctreeAsyncBuilder(uint8 OctreeDepth, const FIntBox& WorldBounds);

public:
	void Init(const FVoxelRenderOctreeSettings& OctreeSettings, TVoxelSharedPtr<FVoxelRenderOctree> Octree);
	void ReportBuildTime();

private:
	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() override;
	virtual uint32 GetPriority() const override;
	//~ End FVoxelAsyncWork Interface

private:
	const uint8 OctreeDepth;
	const FIntBox WorldBounds;

	FVoxelRenderOctreeSettings OctreeSettings{};

	bool bTooManyChunks = false;
	double Counter = 0;
	FString Log;
	int32 NumberOfChunks = 0;
};

class FVoxelRenderOctree : public TSimpleVoxelOctree<RENDER_CHUNK_SIZE, FVoxelRenderOctree>
{
private:
	// Important: must be the first variable to be initialized, else GetId does the wrong thing for the root!
	uint64 RootIdCounter = 0;
	
public:
	FVoxelRenderOctree* const Root;
	const uint64 ChunkId;
	const FIntBox OctreeBounds;

	enum class EDivisionType : uint8
	{
		Uninitialized = 0,
		ByDistance    = 1,
		ByNeighbors   = 2,
		ByOthers      = 3
	};

	struct FChunkSettings
	{
		FVoxelChunkSettings Settings{};
		EDivisionType DivisionType = EDivisionType::Uninitialized;
		EDivisionType OldDivisionType = EDivisionType::Uninitialized;
	}; 
	FChunkSettings ChunkSettings;
	int32 CurrentChunksCount = 0;
	uint64 UpdateIndex = 0;

	inline const FVoxelChunkSettings& GetSettings() const { return ChunkSettings.Settings; }

	FVoxelRenderOctree(uint8 LOD);
	FVoxelRenderOctree(const FVoxelRenderOctree* Source);

	FVoxelRenderOctree(const FVoxelRenderOctree& Parent, uint8 ChildIndex);
	FVoxelRenderOctree(const FVoxelRenderOctree& Parent, uint8 ChildIndex, const ChildrenArray& SourceChildren);

	~FVoxelRenderOctree();

	void ResetDivisionType();
	bool UpdateSubdividedByDistance(const FVoxelRenderOctreeSettings& Settings);
	bool UpdateSubdividedByNeighbors(const FVoxelRenderOctreeSettings& Settings);
	void ReuseOldNeighbors();
	void UpdateSubdividedByOthers(const FVoxelRenderOctreeSettings& Settings);
	void DeleteChunks(TArray<FVoxelChunkUpdate>& ChunkUpdates);

	void GetUpdates(
		uint32 InUpdateIndex,
		bool bRecomputeTransitionMasks,
		const FVoxelRenderOctreeSettings& Settings, 
		TArray<FVoxelChunkUpdate>& ChunkUpdates, 
		bool bVisible = true);

	void GetChunksToUpdateForBounds(const FIntBox& Bounds, TArray<uint64>& ChunksToUpdate, const FVoxelOnChunkUpdate& OnChunkUpdate) const;
	void GetVisibleChunksOverlappingBounds(const FIntBox& Bounds, TArray<uint64, TInlineAllocator<8>>& VisibleChunks) const;

	bool IsCanceled() const;

private:
	bool ShouldSubdivideByDistance(const FVoxelRenderOctreeSettings& Settings) const;
	bool ShouldSubdivideByNeighbors(const FVoxelRenderOctreeSettings& Settings) const;
	bool ShouldSubdivideByOthers(const FVoxelRenderOctreeSettings& Settings) const;
	
	const FVoxelRenderOctree* GetVisibleAdjacentChunk(EVoxelDirection::Type Direction, int32 Index) const;
	
	uint64 GetId();
};