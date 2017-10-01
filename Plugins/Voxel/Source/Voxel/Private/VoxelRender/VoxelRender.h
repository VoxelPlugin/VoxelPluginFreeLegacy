// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include <list>

class AVoxelWorld;
class FChunkOctree;
class AVoxelChunk;

/**
 *
 */
class FVoxelRender
{
public:
	AVoxelWorld* const World;

	FQueuedThreadPool* const MeshThreadPool;
	FQueuedThreadPool* const FoliageThreadPool;


	FVoxelRender(AVoxelWorld* World, uint32 MeshThreadCount, uint32 FoliageThreadCount);


	void Tick(float DeltaTime);

	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	AVoxelChunk* GetInactiveChunk();
	void SetChunkAsInactive(AVoxelChunk* Chunk);

	void UpdateChunk(TWeakPtr<FChunkOctree> Chunk, bool bAsync);
	void UpdateChunksAtPosition(FIntVector Position, bool bAsync);
	void ApplyUpdates();

	void UpdateAll(bool bAsync);

	void UpdateLOD();

	void AddFoliageUpdate(AVoxelChunk* Chunk);

	void AddTransitionCheck(AVoxelChunk* Chunk);

	void AddApplyNewMesh(AVoxelChunk* Chunk);
	void AddApplyNewFoliage(AVoxelChunk* Chunk);

	TWeakPtr<FChunkOctree> GetChunkOctreeAt(FIntVector Position) const;

	int GetDepthAt(FIntVector Position) const;
private:

	// Chunks waiting for update
	TSet<TWeakPtr<FChunkOctree>> ChunksToUpdate;
	// Ids of the chunks that need to be updated synchronously
	TSet<uint64> IdsOfChunksToUpdateSynchronously;

	// Shared ptr because each ChunkOctree need a reference to itself, and the Main one isn't the child of anyone
	TSharedPtr<FChunkOctree> MainOctree;

	std::forward_list<AVoxelChunk*> InactiveChunks;
	TSet<AVoxelChunk*> ActiveChunks;

	TSet<AVoxelChunk*> FoliageUpdateNeeded;

	TSet<AVoxelChunk*> ChunksToCheckForTransitionChange;

	TSet<AVoxelChunk*> ChunksToApplyNewMesh;
	FCriticalSection ChunksToApplyNewMeshLock;

	TSet<AVoxelChunk*> ChunksToApplyNewFoliage;
	FCriticalSection ChunksToApplyNewFoliageLock;

	// Invokers
	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> VoxelInvokerComponents;


	float TimeSinceMeshUpdate;
	float TimeSinceFoliageUpdate;

	void ApplyFoliageUpdates();
	void ApplyTransitionChecks();
	void ApplyNewMeshes();
	void ApplyNewFoliages();
};
