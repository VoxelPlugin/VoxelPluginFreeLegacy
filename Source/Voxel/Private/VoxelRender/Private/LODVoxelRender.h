// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelRender.h"
#include "AsyncWork.h"
#include "VoxelGlobals.h"
#include "VoxelThread.h"
#include "CollisionVoxelRender.h"
#include "VoxelPolygonizer.h"

#define SECTIONS_PER_CHUNK (CHUNK_MULTIPLIER * CHUNK_MULTIPLIER * CHUNK_MULTIPLIER)

class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;
class FVoxelChunkOctree;
class FLODVoxelRender;
class FVoxelChunkToDelete;

struct FVoxelChunkToRemove
{
	float Time;
	UVoxelProceduralMeshComponent* Mesh;
	bool bCollisions;
};

class FAsyncOctreeBuilderTask
{
public:
	TSet<FIntBox> ChunksToDelete;
	TSet<FIntBox> ChunksToCreate;
	TSharedPtr<FVoxelChunkOctree> NewOctree;
	TSharedPtr<FVoxelChunkOctree> OldOctree;
	TMap<FIntBox, uint8> TransitionsMasks;

	FAsyncOctreeBuilderTask(const TArray<FIntBox>& CameraBounds, uint8 LOD, uint8 LODLimit, TSharedPtr<FVoxelChunkOctree> Octree);

	void DoWork();

	bool CanAbandon();
	void Abandon();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncOctreeBuilderTask, STATGROUP_ThreadPoolAsyncTasks);
	};

private:
	TSharedPtr<FVoxelChunkOctree> Octree;
	const uint8 LOD;
	const uint8 LODLimit;
	const TArray<FIntBox> CameraBounds;
};

///////////////////////////////////////////////////////////////////////////////

class FVoxelRenderChunk
{
public:
	FLODVoxelRender* const Render;
	// Minimal corner
	const FIntVector Position;
	const uint8 LOD;
	const FIntBox Bounds;

	FVoxelRenderChunk(FLODVoxelRender* Render, const FIntVector& Position, uint8 LOD, const FIntBox& Bounds);

	void AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk);

	void EndTasks();
	void UpdateChunk(const FIntBox& Box);
	void UpdateTransitions(uint8 NewTransitionsMask);
	void UpdateTransitions();

	void Tick();

	FORCEINLINE UVoxelProceduralMeshComponent* GetMesh() const;
	FORCEINLINE FIntBox GetBounds() const;
	FORCEINLINE int TotalSize() const;
	FORCEINLINE int SectionSize() const;
	FORCEINLINE bool ComputeCollisions() const;
	const TArray<TSharedRef<FVoxelChunkToDelete>>& GetPreviousChunks() const;

private:
	TSharedPtr<FAsyncPolygonizerForTransitionsWork> TransitionsTask;
	
	uint8 TransitionsCurrentMask;
	uint8 TransitionsDisplayedMask;
	uint8 TransitionsMask;
	TArray<uint8, TFixedAllocator<SECTIONS_PER_CHUNK>> ChunksCurrentMask;
	
	UVoxelProceduralMeshComponent* Mesh;	
	TArray<FVoxelIntermediateChunk,							  TFixedAllocator<SECTIONS_PER_CHUNK>> IntermediateChunks;

	TArray<FIntBox,							  TFixedAllocator<SECTIONS_PER_CHUNK>> SectionsBounds;
	TArray<bool,							  TFixedAllocator<SECTIONS_PER_CHUNK>> InitialUpdatingFinished;
	TArray<bool,							  TFixedAllocator<SECTIONS_PER_CHUNK>> SectionNeedsUpdate;
	TArray<TSharedPtr<FAsyncPolygonizerWork>, TFixedAllocator<SECTIONS_PER_CHUNK>> Tasks;
	
	TArray<TSharedRef<FVoxelChunkToDelete>> PreviousChunks;

	void UpdateSection(int SectionIndex);
};

///////////////////////////////////////////////////////////////////////////////

class FVoxelChunkToDelete
{
public:
	FVoxelChunkToDelete(const FVoxelRenderChunk& OldChunk);

	void AddRef();
	void RemoveRef();

	int GetRefCount() const;

	FVoxelChunkToDelete(const FVoxelChunkToDelete&) = delete;
	FVoxelChunkToDelete& operator=(const FVoxelChunkToDelete&) = delete;

private:
	// For recursive deletion
	TArray<TSharedRef<FVoxelChunkToDelete>> OldChunks;
	int RefCount;
	UVoxelProceduralMeshComponent* const Mesh;
	FLODVoxelRender* const Render;
	bool const bCollisions;
};

class FLODVoxelRender : public FCollisionVoxelRender
{

public:
	FVoxelQueuedThreadPool* const MeshThreadPool;

	FLODVoxelRender(AVoxelWorld* World, AActor* ChunksOwner);
	~FLODVoxelRender() override;

	virtual void Tick(float DeltaTime) override;
	virtual void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker) override;
	virtual void UpdateBoxInternal(const FIntBox& Box) override;
	virtual uint8 GetLODAtPosition(const FIntVector& Position) const override;

	void RemoveMesh(UVoxelProceduralMeshComponent* Mesh, bool bCollisions);
	UVoxelProceduralMeshComponent* GetNewMesh(const FIntVector& Position, bool bCollisions, bool bInitialFade, uint8 LOD);


	void AddTaskToDelete(const TSharedPtr<FVoxelAsyncWork>& NewTaskToDelete);

private:
	FQueuedThreadPool* const OctreeBuilderThreadPool;

	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesNoCollisions;
	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesCollisions;
	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;

	TMap<FIntBox, TSharedPtr<FVoxelRenderChunk>> Chunks;
	TArray<TSharedPtr<FVoxelRenderChunk>> ChunksArray;

	TSharedPtr<FVoxelChunkOctree> Octree;
	
	TSharedPtr<FAsyncTask<FAsyncOctreeBuilderTask>> OctreeBuilder;

	TArray<FVoxelChunkToRemove> MeshToRemove;

	TArray<TSharedPtr<FVoxelAsyncWork>> TasksToDelete;

	float TimeSinceUpdate;

	void UpdateLOD();
};