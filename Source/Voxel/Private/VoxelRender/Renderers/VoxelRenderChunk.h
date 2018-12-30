// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

class FVoxelChunkToDelete;
class FVoxelLODRenderer;
class FVoxelPolygonizerAsyncWork;
class FVoxelTransitionsPolygonizerAsyncWork;
class FVoxelMCTransitionsPolygonizerAsyncWork;
class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;


class FVoxelRenderChunk : public IVoxelAsyncCallback
{
public:
	FVoxelLODRenderer* const Render;
	// Minimal corner
	const FIntVector Position;
	const uint8 LOD;
	const FIntBox Bounds;
	
public:
	FVoxelRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds);
	virtual ~FVoxelRenderChunk() { check(bDestroyed); }

public:
	void Destroy();
	void AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk);
public:
	void ResetAlpha();
	void RecreateMaterials();
	void RecomputeMeshPosition();

public:
	void SetScalarParameterValue(FName ParameterName, float Value);
	void SetTextureParameterValue(FName ParameterName, UTexture* Value);
	void SetVectorParameterValue(FName ParameterName, FLinearColor Value);
	
public:
	void UpdateChunk(uint64 TaskId);
	void UpdateTransitions(uint8 NewTransitionsMask);
	
public:
	void Tick();
	void ScheduleTick();

	virtual void OnCallback() override
	{
		ScheduleTick();
	}
	
public:
	inline UVoxelProceduralMeshComponent* GetMesh() const { return Mesh; }
	inline FIntBox GetBounds() const { check(Bounds == FIntBox(Position, Position + FIntVector(TotalSize(), TotalSize(), TotalSize()))); return Bounds; }
	inline int TotalSize() const { return CHUNK_SIZE << LOD; }
	inline const TArray<TSharedRef<FVoxelChunkToDelete>>& GetPreviousChunks() const { return PreviousChunks; }
	inline bool IsInitialLoad() const { return PreviousChunks.Num() > 0; }
	inline uint64 GetTaskId() const { return TaskId; }
	inline uint64 GetQueuedTaskId() const { return QueuedTaskId; }
	bool IsDone() const;

protected:
	virtual TUniquePtr<FVoxelPolygonizerAsyncWork> GetNewTask(const FVoxelPreviousGrassInfo& InPreviousGrassInfo) const = 0;
	virtual TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> GetNewTransitionTask(uint8 InTransitionsMask) const = 0;
	
private:	
	UVoxelProceduralMeshComponent* Mesh = nullptr;

	FVoxelPreviousGrassInfo PreviousGrassInfo;
	
	TSharedRef<FVoxelChunkMaterials> ChunkMaterials;
	TArray<TSharedRef<FVoxelChunkToDelete>> PreviousChunks; // Will be reset after first update

	bool bNeedUpdate = false;
	bool bDestroyed = false; // Only for assert

	TUniquePtr<FVoxelPolygonizerAsyncWork> MeshTask;
	TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> TransitionsTask;
	
	TSharedPtr<FVoxelChunk> MainChunk;
	TSharedPtr<FVoxelChunk> TransitionChunk;
	
	// Transitions
	uint8 TransitionsMaskBeingComputed = 0;
	uint8 TransitionsDisplayedMask = 0;
	uint8 WantedTransitionsMask = 0;
	uint8 MeshDisplayedMask = 0;
	bool bTransitionsNeedUpdate = false;

	uint64 TaskId = 0;
	uint64 QueuedTaskId = 0;

	void Update();
	void UpdateTransitions();
	
	void UpdateMeshFromChunks();
	void EndTasks();

};
