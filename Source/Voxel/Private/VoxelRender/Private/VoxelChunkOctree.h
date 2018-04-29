// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Octree.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"

/**
 * Create the octree for rendering and spawning VoxelChunks
 */
class FVoxelChunkOctree : public TVoxelOctree<FVoxelChunkOctree, RENDER_CHUNK_SIZE>
{
public:
	FVoxelChunkOctree* const Root;
	const TArray<FIntBox> CameraBounds;
	const uint8 LODLimit;

	FVoxelChunkOctree(const TArray<FIntBox>& CameraBounds, uint8 LOD, uint8 LODLimit);
	FVoxelChunkOctree(FVoxelChunkOctree* Parent, uint8 ChildIndex);

	void GetLeavesBounds(TSet<FIntBox>& Bounds) const;
	void GetLeavesTransitionsMasks(TMap<FIntBox, uint8>& TransitionsMasks) const;

	FVoxelChunkOctree* GetAdjacentChunk(EVoxelDirection Direction) const;

private:
	bool bContinueInit;

	void Init();
};