// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include <list>

class AVoxelWorld;
class ChunkOctree;
class AVoxelChunk;

/**
 *
 */
class VoxelRender
{
public:
	AVoxelWorld* const World;

	FQueuedThreadPool* const MeshThreadPool;
	FQueuedThreadPool* const FoliageThreadPool;

	VoxelRender(AVoxelWorld* World, uint32 MeshThreadCount, uint32 FoliageThreadCount);

	void Tick();

	AVoxelChunk* GetInactiveChunk();
	void SetChunkAsInactive(AVoxelChunk* Chunk);

	void UpdateChunk(TWeakPtr<ChunkOctree> Chunk, bool bAsync);
	void UpdateChunksAtPosition(FIntVector Position, bool bAsync);

	void ApplyUpdates();

private:

	// Chunks waiting for update
	TSet<TWeakPtr<ChunkOctree>> QueuedChunks;
	// Ids of the chunks that need to be updated synchronously
	TSet<uint64> IdsOfChunksToUpdateSynchronously;

	// Shared ptr because each ChunkOctree need a reference to itself, and the Main one isn't the child of anyone
	TSharedPtr<ChunkOctree> MainOctree;

	std::forward_list<AVoxelChunk*> InactiveChunks;
	TSet<AVoxelChunk*> ActiveChunks;

	// Invokers
	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> VoxelInvokerComponents;
};
