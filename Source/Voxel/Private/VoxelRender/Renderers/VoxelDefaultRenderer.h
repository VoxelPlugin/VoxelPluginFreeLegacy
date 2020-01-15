// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelMesherAsyncWork.h"
#include "VoxelRender/VoxelChunkToUpdate.h"
#include "VoxelRendererMeshHandler.h"
#include "VoxelTickable.h"
#include "QueueWithNum.h"

class IVoxelRendererMeshHandler;
struct FVoxelChunkMesh;

class FVoxelDefaultRenderer : public IVoxelRenderer, public FVoxelTickable, public TVoxelSharedFromThis<FVoxelDefaultRenderer>
{
public:
	static TVoxelSharedRef<FVoxelDefaultRenderer> Create(const FVoxelRendererSettings& Settings);
	virtual ~FVoxelDefaultRenderer() override;

private:
	explicit FVoxelDefaultRenderer(const FVoxelRendererSettings& Settings);

public:
	//~ Begin IVoxelRender Interface
	virtual int32 UpdateChunks(const FIntBox& Bounds, const TArray<uint64>& ChunksToUpdate, const FVoxelOnChunkUpdateFinished& FinishDelegate) override;
	virtual int32 GetTaskCount() const override;
	virtual void RecomputeMeshPositions() override;
	virtual void ApplyNewMaterials() override;
	virtual void UpdateLODs(uint64 InUpdateIndex, const TArray<FVoxelChunkUpdate>& ChunkUpdates) override;
	virtual void Destroy() override;
	virtual void CreateGeometry_AnyThread(
		int32 LOD,
		const FIntVector& ChunkPosition,
		TArray<uint32>& OutIndices,
		TArray<FVector>& OutVertices) const override;
	//~ End IVoxelRender Interface

	//~ Begin FVoxelTickable Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	//~ End FVoxelTickable Interface

private:
	enum class EChunkState : uint8
	{
		NewChunk,
		Hidden,
		DitheringIn,
		Showed,
		WaitingForNewChunks,
		DitheringOut
	};
	enum class EMainOrTransitions : uint8
	{
		Main,
		Transitions
	};
	struct FChunk
	{
		const uint64 Id;
		const uint8 LOD;
		const FIntBox Bounds;

		FChunk(uint64 Id, uint8 LOD, const FIntBox& Bounds)
			: Id(Id)
			, LOD(LOD)
			, Bounds(Bounds)
		{
		}

		struct FChunkTasks
		{
			TUniquePtr<FVoxelMesherAsyncWork> MainTask;
			TUniquePtr<FVoxelMesherAsyncWork> TransitionsTask;
		};
		FChunkTasks Tasks;

		struct FChunkBuiltData
		{
			uint8 TransitionsMask = 0;
			double MainChunkCreationTime = 0;
			double TransitionsChunkCreationTime = 0;
			TVoxelSharedPtr<const FVoxelChunkMesh> MainChunk;
			TVoxelSharedPtr<const FVoxelChunkMesh> TransitionsChunk;
		};
		FChunkBuiltData BuiltData;

		IVoxelRendererMeshHandler::FChunkId MeshId;

		EChunkState State = EChunkState::NewChunk;

		// Settings to be applied once eg new chunks are spawned
		FVoxelChunkSettings PendingSettings{};
		// Current settings
		FVoxelChunkSettings Settings{};

		struct FPendingUpdate
		{
			// We want the mesh to be from a task that was built >= at this time
			double WantedUpdateTime = 0;
			// Callback once we have a mesh recent enough
			FVoxelOnChunkUpdateFinished OnUpdateFinished;
		};
		TArray<FPendingUpdate, TInlineAllocator<2>> PendingUpdates;

		// Chunks that were shown at this position before this one was shown, and that need to be dithered out
		// once this chunk is updated
		TArray<uint64, TInlineAllocator<8>> PreviousChunks;
		// Number of new chunks left to update
		int32 NumNewChunksLeft = 0;

		// Set when the chunk is being shown
		// This is needed to track if chunks in PreviousChunks are still valid and haven't been switched back to Show
		// Else we'd be decreasing a wrong NumNewChunksLeft
		uint64 UpdateIndex = 0;
	};
	TMap<uint64, FChunk> ChunksMap;

	struct FChunkToRemove
	{
		uint64 Id = 0;
		double Time = 0; // Time at which to remove the chunk
	};
	TArray<FChunkToRemove> ChunksToRemove;
	
	struct FChunkToShow
	{
		uint64 Id = 0;
		double Time = 0; // Time at which to stop dithering the chunk
	};
	TArray<FChunkToShow> ChunksToShow;

	TArray<IVoxelQueuedWork*> QueuedTasks[2][2]; // [bVisible][bHasCollisions]

	enum class EIfTaskExists : uint8
	{
		DoNothing,
		Assert
	};

	template<EMainOrTransitions MainOrTransitions, EIfTaskExists IfTaskExists>
	void StartTask(FChunk& Chunk);
	void CancelTasks(FChunk& Chunk);
	void ClearPreviousChunks(FChunk& Chunk);
	void NewChunksFinished(FChunk& Chunk, const FChunk& NewChunk);
	void RemoveOrHideChunk(FChunk& Chunk);
	void DitherInChunk(FChunk& Chunk, const TArray<uint64, TInlineAllocator<8>>& PreviousChunks);
	void ApplyPendingSettings(FChunk& Chunk);
	void CheckPendingUpdates(FChunk& Chunk);
	void ProcessChunksToRemoveOrShow();
	void FlushQueuedTasks();
	void DestroyChunk(FChunk& Chunk);
	
private:
	// Need shared ptr for async callbacks
	TVoxelSharedPtr<IVoxelRendererMeshHandler> MeshHandler;
	
	FThreadSafeCounter TaskCount;
	uint64 UpdateIndex = 0;
	bool OnWorldLoadedFired = false;

#if VOXEL_DEBUG
	TMap<uint64, FVoxelChunkSettings> DebugChunks;
#endif

private:
	uint32 AllocatedSize = 0;

	void UpdateAllocatedSize();

public:
	void QueueChunkCallback_AnyThread(uint64 TaskId, uint64 ChunkId, bool bIsTransitionTask);

private:
	struct FVoxelTaskCallback
	{
		uint64 TaskId;
		uint64 ChunkId;
		bool bIsTransitionTask;
	};
	TQueueWithNum<FVoxelTaskCallback, EQueueMode::Mpsc> TasksCallbacksQueue;

	void CancelTask(TUniquePtr<FVoxelMesherAsyncWork>& Task);
};