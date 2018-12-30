// Copyright 2018 Phyronnaz

#pragma once

#include "VoxelRender/IVoxelRender.h"
#include "Async/AsyncWork.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRenderChunk.h"

class UHierarchicalInstancedStaticMeshComponent;
class UVoxelProceduralMeshComponent;
class FVoxelChunkOctree;
class FVoxelLODRenderer;
class FVoxelChunkToDelete;
class FVoxelPool;
class IVoxelQueuedWork;

struct FVoxelChunkToRemove
{
	float TimeOfDeletion;
	UVoxelProceduralMeshComponent* Mesh;
	bool bCollisions;
	FIntBox Bounds;
};

class FAsyncOctreeBuilderTask : public IQueuedWork
{
public:
	TSet<FIntBox> ChunksToDelete;
	TSet<FIntBox> ChunksToCreate;
	TSharedPtr<FVoxelChunkOctree> NewOctree;
	TSharedPtr<FVoxelChunkOctree> OldOctree;
	TMap<FIntBox, uint8> TransitionsMasks;

	FAsyncOctreeBuilderTask(uint8 LOD, const FIntBox& WorldBounds);

public:
	void Init(
		const TArray<FIntVector>& CameraPositions,
		TSharedPtr<FVoxelChunkOctree> Octree,
		const TMap<uint8, float>& LODToMinDistance,
		float VoxelSize,
		uint8 LODLimit);
	void Reset();
	bool IsActive() const { return bIsActive; }

public:
	void DoWork();

public:
	void DoThreadedWork() override;
	void Abandon() override;

	bool IsDone() const;
	void WaitForCompletion();
	void Autodelete();

private:
	FThreadSafeCounter IsDoneCounter;
	FCriticalSection DoneSection;
	bool bIsActive = false;
	bool bAutodelete = false;

private:
	const uint8 LOD;
	const FIntBox WorldBounds;

	TArray<FIntVector> CameraPositions;
	TSharedPtr<FVoxelChunkOctree> Octree;
	TMap<uint8, float> LODToMinDistance;
	float VoxelSize = 0;
	uint8 LODLimit = 0;

};

///////////////////////////////////////////////////////////////////////////////

class FVoxelChunkToDelete
{
public:
	FVoxelChunkToDelete(FVoxelRenderChunk& OldChunk);
	~FVoxelChunkToDelete();

	FVoxelChunkToDelete(const FVoxelChunkToDelete&) = delete;
	FVoxelChunkToDelete& operator=(const FVoxelChunkToDelete&) = delete;

private:
	FVoxelLODRenderer* const Render;
	const FIntBox Bounds;

	// For recursive deletion
	TArray<TSharedRef<FVoxelChunkToDelete>> OldChunks;
	
	UVoxelProceduralMeshComponent* const Mesh;
};

///////////////////////////////////////////////////////////////////////////////

struct FVoxelTasksDependenciesHandler
{
public:
	struct FLockedTask
	{
		TWeakPtr<FVoxelRenderChunk> Chunk;
		uint64 Id;
		template<typename T> FLockedTask(T& Chunk, uint64 Id) : Chunk(Chunk), Id(Id) {}
	};
	struct FGroup
	{
		TArray<FLockedTask> Tasks;
		TArray<TFunction<void()>> CallbacksWhenUpdated;
		bool bValid = true;

		FGroup() = default;
		FGroup(TArray<FLockedTask>&& Tasks, TFunction<void()> CallbackWhenUpdated) : Tasks(Tasks), CallbacksWhenUpdated({ CallbackWhenUpdated }) {}
	};

	inline uint64 GetTaskId() { return TaskIdCounter++; }

	inline void AddGroup(TArray<FLockedTask>&& Tasks, TFunction<void()> CallbackWhenUpdated)
	{
		TArray<TSharedPtr<FGroup>> GroupsToMerge;
		for (auto& Task : Tasks)
		{
			auto* Result = TaskGroupsMap.Find(Task.Id);
			if (Result && Result->IsValid())
			{
				auto GroupToMerge = Result->Pin();
				TasksGroups.Remove(GroupToMerge);
				GroupsToMerge.Add(GroupToMerge);
			}
		}
		GroupsToMerge.Add(MakeShared<FGroup>(MoveTemp(Tasks), CallbackWhenUpdated));

		TSharedPtr<FGroup> FinalGroup = MakeShared<FGroup>();
		TMap<TWeakPtr<FVoxelRenderChunk>, uint64> ChunkIds;
		for (auto& GroupToMerge : GroupsToMerge)
		{
			for (auto& Task : GroupToMerge->Tasks)
			{
				auto& Id = ChunkIds.FindOrAdd(Task.Chunk);
				if (Id == 0 || Id > Task.Id) { Id = Task.Id; }
			}
			FinalGroup->CallbacksWhenUpdated.Append(MoveTemp(GroupToMerge->CallbacksWhenUpdated));
		}
		for (auto& It : ChunkIds)
		{
			FinalGroup->Tasks.Emplace(It.Key, It.Value);
			TaskGroupsMap.Add(It.Value, FinalGroup);
		}
		TasksGroups.Add(FinalGroup);
	}
	inline const TArray<TSharedPtr<FGroup>>& GetTaskGroups() const { return TasksGroups; }
	inline bool CanApplyTask(uint64 TaskId) const { auto* Ptr = TaskGroupsMap.Find(TaskId); return !Ptr || !Ptr->IsValid(); }
	inline void ClearGroup(const TSharedPtr<FGroup>& Group)
	{
		for (auto& Task : Group->Tasks)
		{
			TaskGroupsMap.Remove(Task.Id);
		}
		for (auto& Callback : Group->CallbacksWhenUpdated)
		{
			if (Callback)
			{
				Callback();
			}
		}
		Group->bValid = false;
	}
	inline void CleanGroups()
	{		
		TasksGroups.RemoveAll([](auto& T) { return !T->bValid; });
	}
private:
	TArray<TSharedPtr<FGroup>> TasksGroups;
	TMap<uint64, TWeakPtr<FGroup>> TaskGroupsMap;
	uint64 TaskIdCounter = 1;
};
///////////////////////////////////////////////////////////////////////////////

class FVoxelLODRenderer : public IVoxelRender
{
public:
	FVoxelLODRenderer(AVoxelWorld* World);
	~FVoxelLODRenderer() override;

public:
	//~ Begin IVoxelRender Interface
	virtual void Tick(float DeltaTime) override;
	virtual void UpdateBoxInternal(const FIntBox& Box, bool bRemoveHoles, TFunction<void()> CallbackWhenUpdated) override;
	virtual uint8 GetLODAtPosition(const FIntVector& Position) const override;
	virtual int GetTaskCount() const override { return bOctreeBuilt ? TaskCount : -1; }
	virtual	void RecreateMaterials() override;
	virtual void RecomputeMeshPositions() override;
	
	virtual void SetScalarParameterValue(FName ParameterName, float Value) override;
	virtual void SetTextureParameterValue(FName ParameterName, UTexture* Value) override;
	virtual void SetVectorParameterValue(FName ParameterName, FLinearColor Value) override;
	//~ End IVoxelRender Interface
	
	//~ Begin FVoxelLODRenderer Interface
	virtual TSharedRef<FVoxelRenderChunk> GetRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds) = 0;
	//~ End FVoxelLODRenderer Interface

public:
	void RemoveMesh(UVoxelProceduralMeshComponent* Mesh, const FIntBox& Bounds);
	UVoxelProceduralMeshComponent* GetNewMesh(const FIntVector& Position, uint8 LOD);

public:
	void ScheduleTick(FVoxelRenderChunk* Chunk);
	void AddQueuedWork(IVoxelQueuedWork* QueuedWork);
	bool RetractQueuedWork(IVoxelQueuedWork* QueuedWork);

	inline void IncreaseTaskCount() { TaskCount++; }
	inline void DecreaseTaskCount() { TaskCount--; check(TaskCount >= 0); }
	
	inline bool CanApplyTask(uint64 TaskId) const { return DependenciesHandler.CanApplyTask(TaskId); }
	
	uint64 GetSquaredDistanceToInvokers(const FIntVector& Position) const;
	
private:
	FVoxelPool* const Pool;

	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesNoCollisions;
	TArray<UVoxelProceduralMeshComponent*> InactiveMeshesCollisions;

	TMap<FIntBox, TSharedPtr<FVoxelRenderChunk>> Chunks;
	TSharedPtr<FVoxelChunkOctree> Octree;	
	TUniquePtr<FAsyncOctreeBuilderTask> OctreeBuilder;
	TArray<FVoxelChunkToRemove> ChunkToRemove;
	float NextUpdateTime = 0;
	// Can't use game time in data asset editor
	float WorldTime = 0;

	int TaskCount = 0;
	bool bOctreeBuilt = false;

	TSet<FVoxelRenderChunk*> ChunksToTick;
	FCriticalSection ChunksToTickLock;

	FVoxelTasksDependenciesHandler DependenciesHandler;

	void UpdateLOD();
	float GetWorldTime() const;
};
