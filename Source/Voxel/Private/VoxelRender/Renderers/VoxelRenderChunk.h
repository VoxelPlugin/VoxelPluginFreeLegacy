// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "Engine/EngineTypes.h"

class FVoxelChunkToDelete;
class FVoxelDefaultRenderer;
class FVoxelPolygonizerAsyncWork;
class FVoxelTransitionsPolygonizerAsyncWork;
class FVoxelMCTransitionsPolygonizerAsyncWork;
class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;

class FVoxelRenderChunk : public TSharedFromThis<FVoxelRenderChunk, ESPMode::ThreadSafe>
{
public:
	FVoxelDefaultRenderer* const Renderer;
	// Minimal corner
	const FIntVector Position;
	const uint8 LOD;
	const FIntBox Bounds;
	FVoxelRenderChunkSettings Settings;

public:
	FVoxelRenderChunk(FVoxelDefaultRenderer* Renderer, uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& Settings);
	virtual ~FVoxelRenderChunk();

public:
	void Destroy();
	void AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk);

public:
	void CancelDithering();
	void RecomputeMeshPosition();
	void UpdateSettings(const FVoxelRenderChunkSettings& NewSettings);
	
public:
	void UpdateChunk(uint64 TaskId, const FVoxelOnUpdateFinished& Delegate = FVoxelOnUpdateFinished());
	void UpdateTransitions(uint8 NewTransitionsMask);
	void UpdateTransitions();
	
public:
	void MeshCallback(uint64 TaskId);
	void TransitionsCallback();
	void WaitForDependenciesCallback(bool bTimeout);
	void NewChunksAreUpdated(); // Called when bVisible is set to false & new chunks are updated (can start dithering)
	void HideMeshAfterDithering(); // Called when dithering is finished
	
public:
	inline UVoxelProceduralMeshComponent* GetMesh() const { return Mesh; }
	inline const FIntBox& GetBounds() const { check(Bounds == FIntBox(Position, Position + FIntVector(TotalSize(), TotalSize(), TotalSize()))); return Bounds; }
	inline int32 TotalSize() const { return CHUNK_SIZE << LOD; }
	inline bool IsInitialLoad() const { return PreviousChunks.Num() > 0; }
	inline uint64 GetTaskId() const { return TaskId; }
	inline uint8 GetWantedTransitionsMask() const { return WantedTransitionsMask; }
	inline bool NeedsToBeDeleted() const { return Mesh || PreviousChunks.Num() > 0; }
	inline bool IsDestroyed() const { return bDestroyed; }
	bool CanStartUpdateWithCustomTaskId() const;
	uint64 GetPriority() const;
	
	inline TArray<TSharedRef<FVoxelChunkToDelete>>&& MovePreviousChunks() { return MoveTemp(PreviousChunks); }

protected:
	virtual TUniquePtr<FVoxelPolygonizerAsyncWork> GetNewTask() = 0;
	virtual TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> GetNewTransitionTask() = 0;
	
private:	
	TUniquePtr<FVoxelChunkMaterials> ChunkMaterials;
	UVoxelProceduralMeshComponent* Mesh = nullptr;
	TArray<TSharedRef<FVoxelChunkToDelete>> PreviousChunks; // Will be reset after first update

	bool bUpdateQueued = false;
	bool bDestroyed = false;

	TUniquePtr<FVoxelPolygonizerAsyncWork> MeshTask;
	TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> TransitionsTask;
	
	TSharedPtr<FVoxelChunk> MainChunk;
	TSharedPtr<FVoxelChunk> TransitionsChunk;
	
	// Transitions
	uint8 TransitionsBeingComputedMask = 0;
	uint8 TransitionsChunkMask = 0;
	uint8 WantedTransitionsMask = 0;
	uint8 MeshDisplayedMask = 0;
	bool bTransitionsUpdateQueued = false;

	uint64 TaskId = 0;

	FTimerHandle HideMeshAfterDitheringHandle;

	TArray<FVoxelOnUpdateFinished> OnUpdate;
	TArray<FVoxelOnUpdateFinished> OnQueuedUpdate;


	void StartUpdate();
	void StartTransitionsUpdate();

	bool TryToUpdateFromMeshTask();
	bool TryToUpdateFromTransitionsTask();
	
	void UpdateMeshFromChunks();
	void EndTasks();
};
