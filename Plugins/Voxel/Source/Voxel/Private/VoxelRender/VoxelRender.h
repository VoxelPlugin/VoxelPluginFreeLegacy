// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include <list>

class AVoxelWorld;
class FChunkOctree;
class UVoxelChunk;

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
	~FVoxelRender();


	void Tick(float DeltaTime);

	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	UVoxelChunk* GetInactiveChunk();
	void SetChunkAsInactive(UVoxelChunk* Chunk);

	void UpdateChunk(TWeakPtr<FChunkOctree> Chunk, bool bAsync);
	void UpdateChunksAtPosition(FIntVector Position, bool bAsync);
	void ApplyUpdates();

	void UpdateAll(bool bAsync);

	void UpdateLOD();


	void AddFoliageUpdate(UVoxelChunk* Chunk);
	void AddApplyNewMesh(UVoxelChunk* Chunk);
	void AddApplyNewFoliage(UVoxelChunk* Chunk);

	void RemoveFromQueues(UVoxelChunk* Chunk);

	// Not the same as the queues above, as it is emptied at the same frame: see ApplyUpdates
	void AddTransitionCheck(UVoxelChunk* Chunk);

	TWeakPtr<FChunkOctree> GetChunkOctreeAt(FIntVector Position) const;

	int GetDepthAt(FIntVector Position) const;

	void Delete();

private:

	// Chunks waiting for update
	TSet<TWeakPtr<FChunkOctree>> ChunksToUpdate;
	// Ids of the chunks that need to be updated synchronously
	TSet<uint64> IdsOfChunksToUpdateSynchronously;

	// Shared ptr because each ChunkOctree need a reference to itself, and the Main one isn't the child of anyone
	TSharedPtr<FChunkOctree> MainOctree;

	std::forward_list<UVoxelChunk*> InactiveChunks;
	TSet<UVoxelChunk*> ActiveChunks;

	TSet<UVoxelChunk*> FoliageUpdateNeeded;

	TSet<UVoxelChunk*> ChunksToCheckForTransitionChange;

	TSet<UVoxelChunk*> ChunksToApplyNewMesh;
	FCriticalSection ChunksToApplyNewMeshLock;

	TSet<UVoxelChunk*> ChunksToApplyNewFoliage;
	FCriticalSection ChunksToApplyNewFoliageLock;

	// Invokers
	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> VoxelInvokerComponents;


	float TimeSinceMeshUpdate;
	float TimeSinceFoliageUpdate;

	void ApplyFoliageUpdates();
	void ApplyNewMeshes();
	void ApplyNewFoliages();


	void ApplyTransitionChecks();
};
