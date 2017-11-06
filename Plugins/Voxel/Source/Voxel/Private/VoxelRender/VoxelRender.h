// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelBox.h"
#include <list>

class AVoxelWorld;
class FVoxelData;;
class FChunkOctree;
class UVoxelChunkComponent;

struct FChunkToDelete
{
	UVoxelChunkComponent* Chunk;
	float TimeLeft;

	FChunkToDelete(UVoxelChunkComponent* Chunk, float TimeLeft)
		: Chunk(Chunk)
		, TimeLeft(TimeLeft)
	{
	};
};

/**
 *
 */
class FVoxelRender
{
public:
	AVoxelWorld* const World;
	AActor* const ChunksParent;
	FVoxelData* const Data;

	FQueuedThreadPool* const MeshThreadPool;
	FQueuedThreadPool* const HighPriorityMeshThreadPool;
	FQueuedThreadPool* const FoliageThreadPool;


	FVoxelRender(AVoxelWorld* World, AActor* ChunksParent, FVoxelData* Data, uint32 MeshThreadCount, uint32 HighPriorityMeshThreadCount, uint32 FoliageThreadCount);
	~FVoxelRender();


	void Tick(float DeltaTime);

	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	UVoxelChunkComponent* GetInactiveChunk();

	void UpdateChunk(TWeakPtr<FChunkOctree> Chunk, bool bAsync);
	void UpdateChunksAtPosition(FIntVector Position, bool bAsync);
	void UpdateChunksOverlappingBox(FVoxelBox Box, bool bAsync);
	void ApplyUpdates();

	void UpdateAll(bool bAsync);

	void UpdateLOD();


	void AddFoliageUpdate(UVoxelChunkComponent* Chunk);
	void AddApplyNewMesh(UVoxelChunkComponent* Chunk);
	void AddApplyNewFoliage(UVoxelChunkComponent* Chunk);

	// Not the same as the queues above, as it is emptied at the same frame: see ApplyUpdates
	void AddTransitionCheck(UVoxelChunkComponent* Chunk);


	void ScheduleDeletion(UVoxelChunkComponent* Chunk);
	void ChunkHasBeenDestroyed(UVoxelChunkComponent* Chunk);

	TWeakPtr<FChunkOctree> GetChunkOctreeAt(FIntVector Position) const;

	int GetDepthAt(FIntVector Position) const;

	// MUST be called before delete
	void Destroy();

	// Needed when ChunksParent != World
	FVector GetGlobalPosition(FIntVector LocalPosition);

private:

	// Chunks waiting for update
	TSet<TWeakPtr<FChunkOctree>> ChunksToUpdate;
	// Ids of the chunks that need to be updated synchronously
	TSet<uint64> IdsOfChunksToUpdateSynchronously;

	// Shared ptr because each ChunkOctree need a reference to itself, and the Main one isn't the child of anyone
	TSharedPtr<FChunkOctree> MainOctree;

	std::forward_list<UVoxelChunkComponent*> InactiveChunks;
	TSet<UVoxelChunkComponent*> ActiveChunks;

	TSet<UVoxelChunkComponent*> FoliageUpdateNeeded;

	TSet<UVoxelChunkComponent*> ChunksToCheckForTransitionChange;

	TSet<UVoxelChunkComponent*> ChunksToApplyNewMesh;
	FCriticalSection ChunksToApplyNewMeshLock;

	TSet<UVoxelChunkComponent*> ChunksToApplyNewFoliage;
	FCriticalSection ChunksToApplyNewFoliageLock;

	std::forward_list<FChunkToDelete> ChunksToDelete;

	// Invokers
	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> VoxelInvokerComponents;


	float TimeSinceFoliageUpdate;
	float TimeSinceLODUpdate;

	void RemoveFromQueues(UVoxelChunkComponent* Chunk);
};
