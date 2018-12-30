// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelOctree.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"

struct FRichCurve;

struct FVoxelChunkOctreeSettings
{
	uint8 LODLimit;
	FIntBox WorldBounds;
	TArray<FIntVector> CameraPositions;
	// If Distance < SquaredDistances[LOD], subdivide
	TArray<uint64> SquaredDistances;
};

/**
 * Create the octree for rendering and spawning VoxelChunks
 */
class FVoxelChunkOctree : public TVoxelOctree<FVoxelChunkOctree, CHUNK_SIZE>
{
public:
	const FVoxelChunkOctreeSettings& Settings;
	FVoxelChunkOctree* const Root;

	FVoxelChunkOctree(const FVoxelChunkOctreeSettings* Settings, uint8 LOD);
	FVoxelChunkOctree(FVoxelChunkOctree* Parent, uint8 ChildIndex);

	bool ShouldSubdivide() const;

	void GetLeavesBounds(TSet<FIntBox>& Bounds) const;
	void GetLeavesTransitionsMasks(TMap<FIntBox, uint8>& TransitionsMasks) const;

	FVoxelChunkOctree* GetAdjacentChunk(EVoxelDirection Direction) const;

private:
	bool bContinueInit;

	void Init();
};