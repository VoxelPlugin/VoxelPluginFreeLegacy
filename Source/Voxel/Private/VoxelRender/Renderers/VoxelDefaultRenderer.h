// Copyright 2019 Phyronnaz

#pragma once

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelMathUtilities.h"
#include "Engine/EngineTypes.h"

class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;
class FVoxelDefaultRenderer;
class FVoxelChunkToDelete;
class IVoxelPool;
class IVoxelQueuedWork;
class FVoxelRenderOctreeAsyncBuilder;
class FVoxelRenderChunk;

class FVoxelChunkToRemoveAfterDithering
{
public:
	FVoxelChunkToRemoveAfterDithering(
		TWeakObjectPtr<UVoxelProceduralMeshComponent> Mesh,
		const FIntBox& Bounds,
		TSharedRef<FVoxelDefaultRenderer> Renderer,
		float TimeUntilDeletion,
		const TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& NewRenderChunks);

	~FVoxelChunkToRemoveAfterDithering();

	inline bool IsDone() const { return bIsDone; }
	inline const FIntBox& GetBounds() const { return Bounds; }
	void Remove();
	void SetMeshRelativeLocation(const FVector& Location);

private:
	const TWeakObjectPtr<UVoxelProceduralMeshComponent> Mesh;
	const FIntBox Bounds;
	const TWeakPtr<FVoxelDefaultRenderer> Renderer;
	const TWeakObjectPtr<UWorld> World;
	const TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>> NewRenderChunks;

	FTimerHandle TimerHandle;
	bool bIsDone = false;
};

class FVoxelChunkToDelete
{
public:
	FVoxelChunkToDelete(FVoxelRenderChunk& OldChunk, bool bDontRemoveMeshAndNotifyChunkInstead);
	~FVoxelChunkToDelete();

	template<typename T>
	inline void AddNewChunk(const T& NewChunk) { NewRenderChunks.Add(NewChunk); }

private:
	const TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> OldRenderChunk;
	TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>> NewRenderChunks;

	const bool bDontRemoveMeshAndNotifyChunkInstead;

	const TWeakPtr<FVoxelDefaultRenderer> Render;
	const FIntBox Bounds;
	// For recursive deletion
	const TArray<TSharedRef<FVoxelChunkToDelete>> OldChunksToDelete;
	
	UVoxelProceduralMeshComponent* const Mesh;
};

struct FVoxelTasksDependenciesHandler
{
public:
	struct FLockedTask
	{
		TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> Chunk;
		uint64 Id;

		FLockedTask(TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> Chunk, uint64 Id)
			: Chunk(Chunk)
			, Id(Id)
		{
		}
	};
	struct FGroup
	{
		const TArray<FLockedTask> Tasks;
		TArray<uint64> PendingIds;
		FTimerHandle Handle;

		FGroup() = default;
		FGroup(const TArray<FLockedTask>& Tasks);
	};

	FVoxelTasksDependenciesHandler(TWeakObjectPtr<UWorld> World, float WaitForOtherChunksToAvoidHolesTimeout);
	~FVoxelTasksDependenciesHandler();

	inline uint64 GetTaskId() { return ++TaskIdCounter; }

	void AddGroup(const TArray<FLockedTask>& Tasks);

	bool CanApplyTask(uint64 TaskId) const;
	void ReportTaskFinished(uint64 TaskId);

private:
	const TWeakObjectPtr<UWorld> World;
	const float WaitForOtherChunksToAvoidHolesTimeout;

	TArray<TSharedPtr<FGroup>> TasksGroups;
	TMap<uint64, TWeakPtr<FGroup>> TaskIdsToGroupsMap;
	uint64 TaskIdCounter = 0;

	void RemoveGroup(TWeakPtr<FGroup> Group, bool bTimeout);
};

class FVoxelDefaultRenderer : public IVoxelRenderer, public TSharedFromThis<FVoxelDefaultRenderer>
{
public:
	FVoxelDefaultRenderer(const FVoxelRendererSettings& Settings);
	~FVoxelDefaultRenderer() override;

public:
	//~ Begin IVoxelRender Interface
	virtual void UpdateChunks(const TArray<uint64>& ChunksToUpdate, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate) override;
	virtual void CancelDithering(const FIntBox& Bounds, const TArray<uint64>& Chunks) override;
	virtual int32 GetTaskCount() const override { return bUpdatesStarted ? TaskCount : -1; }
	virtual void RecomputeMeshPositions() override;
	virtual void UpdateLODs(const TArray<FVoxelChunkToAdd>& ChunksToAdd, const TArray<FVoxelChunkToUpdate>& ChunksToUpdate, const TArray<FVoxelChunkToRemove>& ChunksToRemove, const TArray<FVoxelTransitionsToUpdate>& TransitionsToUpdate) override;
	//~ End IVoxelRender Interface
	
	//~ Begin FVoxelLODRenderer Interface
	virtual TSharedRef<FVoxelRenderChunk, ESPMode::ThreadSafe> GetRenderChunk(uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& Settings) = 0;
	//~ End FVoxelLODRenderer Interface

public:
	void StartMeshDithering(UVoxelProceduralMeshComponent* Mesh, const FIntBox& Bounds, const TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& NewRenderChunks);
	void SetMeshPosition(UVoxelProceduralMeshComponent* Mesh, const FIntVector& Position);
	void RemoveMesh(UVoxelProceduralMeshComponent* Mesh);
	UVoxelProceduralMeshComponent* GetNewMesh(const FIntVector& Position, uint8 LOD, bool bCollisions);

public:
	void IncreaseTaskCount();
	void DecreaseTaskCount();
	
	inline bool CanApplyTask(uint64 TaskId) const
	{
		return TaskId == 0 || DependenciesHandler.CanApplyTask(TaskId);
	}
	inline void ReportTaskFinished(uint64 TaskId)
	{
		if (TaskId != 0)
		{
			DependenciesHandler.ReportTaskFinished(TaskId);
		}
	}

	uint64 GetSquaredDistanceToInvokers(const FIntVector& Position) const;
	
private:
	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesNoCollisions;
	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesCollisions;

	inline TArray<UVoxelProceduralMeshComponent*>& GetMeshPool(bool bCollisions)
	{
		return bCollisions ? InactiveMeshesCollisions : InactiveMeshesNoCollisions;
	}

	TMap<uint64, TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>> ChunksMap;
	TArray<TUniquePtr<FVoxelChunkToRemoveAfterDithering>> ChunksToRemoveAfterDithering;

	int32 TaskCount = 0;
	bool bUpdatesStarted = false;
	bool bOnLoadedCallbackFired = false;

	FVoxelTasksDependenciesHandler DependenciesHandler;
};
