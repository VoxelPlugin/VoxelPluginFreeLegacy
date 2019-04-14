// Copyright 2019 Phyronnaz

#include "VoxelDefaultRenderer.h"

#include "VoxelRenderChunk.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelThreadPool.h"
#include "IVoxelPool.h"

#include "Engine/World.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "TimerManager.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "Voxel"

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::~FVoxelDefaultRenderer"), STAT_FVoxelDefaultRenderer_FVoxelDefaultRenderer, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateChunks"), STAT_VoxelDefaultRenderer_UpdateChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::CancelDithering"), STAT_VoxelDefaultRenderer_CancelDithering, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::RecomputeMeshPositions"), STAT_VoxelDefaultRenderer_RecomputeMeshPositions, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs"), STAT_VoxelDefaultRenderer_UpdateLODs, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelChunkToRemoveAfterDithering::FVoxelChunkToRemoveAfterDithering(
		TWeakObjectPtr<UVoxelProceduralMeshComponent> Mesh,
		const FIntBox& Bounds,
		TSharedRef<FVoxelDefaultRenderer> Renderer,
		float TimeUntilDeletion,
		const TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& NewRenderChunks)
	: Mesh(TWeakObjectPtr<UVoxelProceduralMeshComponent>(Mesh))
	, Bounds(Bounds)
	, Renderer(Renderer)
	, World(Renderer->Settings.World)
	, NewRenderChunks(NewRenderChunks)
{
	ensure(Mesh.IsValid());

	if (ensure(World.IsValid()))
	{
		auto& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(TimerHandle, FTimerDelegate::CreateRaw(this, &FVoxelChunkToRemoveAfterDithering::Remove), TimeUntilDeletion, false);
	}
}

FVoxelChunkToRemoveAfterDithering::~FVoxelChunkToRemoveAfterDithering()
{
	if (World.IsValid())
	{
		auto& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
	}
}

void FVoxelChunkToRemoveAfterDithering::Remove()
{
	auto RendererPtr = Renderer.Pin();
	if (ensure(!IsDone()) && ensure(RendererPtr.IsValid()) && Mesh.IsValid())
	{
		RendererPtr->RemoveMesh(Mesh.Get());
	}
	for (auto& WeakNewChunk : NewRenderChunks)
	{
		auto NewChunk = WeakNewChunk.Pin();
		if (NewChunk.IsValid())
		{
			NewChunk->CancelDithering();
		}
	}
	bIsDone = true;
}

void FVoxelChunkToRemoveAfterDithering::SetMeshRelativeLocation(const FVector& Location)
{
	Mesh->SetRelativeLocation(Location);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_MEMORY_STAT(TEXT("Voxel Chunks To Delete Memory"), STAT_VoxelChunksToDeleteMemory, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Chunks To Delete Count"), STAT_VoxelChunksToDeleteCount, STATGROUP_VoxelMemory);

DECLARE_CYCLE_STAT(TEXT("FVoxelChunkToDelete::~FVoxelChunkToDelete"), STAT_FVoxelChunkToDelete_FVoxelChunkToDelete_Destructor, STATGROUP_Voxel);

FVoxelChunkToDelete::FVoxelChunkToDelete(FVoxelRenderChunk& OldRenderChunk, bool bDontRemoveMeshAndNotifyChunkInstead)
	: OldRenderChunk(OldRenderChunk.AsShared())
	, bDontRemoveMeshAndNotifyChunkInstead(bDontRemoveMeshAndNotifyChunkInstead)	
	, Render(OldRenderChunk.Renderer->AsShared())
	, Bounds(OldRenderChunk.GetBounds())
	, OldChunksToDelete(OldRenderChunk.MovePreviousChunks())
	, Mesh(OldRenderChunk.GetMesh())
{
	INC_DWORD_STAT_BY(STAT_VoxelChunksToDeleteCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelChunksToDeleteMemory, sizeof(FVoxelChunkToDelete));
}

FVoxelChunkToDelete::~FVoxelChunkToDelete()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelChunkToDelete_FVoxelChunkToDelete_Destructor);
	DEC_DWORD_STAT_BY(STAT_VoxelChunksToDeleteCount, 1);
	DEC_MEMORY_STAT_BY(STAT_VoxelChunksToDeleteMemory, sizeof(FVoxelChunkToDelete));
	
	auto RenderPtr = Render.Pin();
	if (RenderPtr.IsValid())
	{
		if (bDontRemoveMeshAndNotifyChunkInstead)
		{
			auto OldRenderChunkPtr = OldRenderChunk.Pin();
			if (OldRenderChunkPtr.IsValid())
			{
				OldRenderChunkPtr->NewChunksAreUpdated();
			}
		}
		else
		{
			if (Mesh)
			{
				RenderPtr->StartMeshDithering(Mesh, Bounds, NewRenderChunks);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelTasksDependenciesHandler::FGroup::FGroup(const TArray<FLockedTask>& Tasks)
	: Tasks(Tasks)
{
	for (auto& Task : Tasks)
	{
		PendingIds.Add(Task.Id);
	}
}

FVoxelTasksDependenciesHandler::FVoxelTasksDependenciesHandler(TWeakObjectPtr<UWorld> World, float WaitForOtherChunksToAvoidHolesTimeout)
	: World(World)
	, WaitForOtherChunksToAvoidHolesTimeout(WaitForOtherChunksToAvoidHolesTimeout)
{

}

FVoxelTasksDependenciesHandler::~FVoxelTasksDependenciesHandler()
{
	if (World.IsValid())
	{
		auto& TimerManager = World->GetTimerManager();
		for (auto& Group : TasksGroups)
		{
			TimerManager.ClearTimer(Group->Handle);
		}
	}
}

void FVoxelTasksDependenciesHandler::AddGroup(const TArray<FLockedTask>& Tasks)
{
	auto Group = MakeShared<FGroup>(Tasks);
	TasksGroups.Add(Group);
	for (auto& Task : Tasks)
	{
		ensure(!TaskIdsToGroupsMap.Contains(Task.Id));
		TaskIdsToGroupsMap.Add(Task.Id, Group);
	}
	if (World.IsValid())
	{
		auto& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(Group->Handle, FTimerDelegate::CreateRaw(this, &FVoxelTasksDependenciesHandler::RemoveGroup, TWeakPtr<FGroup>(Group), true), WaitForOtherChunksToAvoidHolesTimeout, false);
	}
}

bool FVoxelTasksDependenciesHandler::CanApplyTask(uint64 TaskId) const
{
	ensure(TaskId != 0);
	auto* Ptr = TaskIdsToGroupsMap.Find(TaskId);
	ensure(!Ptr || Ptr->IsValid());
	return !Ptr;
}

void FVoxelTasksDependenciesHandler::ReportTaskFinished(uint64 TaskId)
{
	ensure(TaskId != 0);
	auto* GroupWeakPtr = TaskIdsToGroupsMap.Find(TaskId);
	if (GroupWeakPtr && GroupWeakPtr->IsValid())
	{
		auto GroupPtr = GroupWeakPtr->Pin();
		auto& Group = *GroupPtr;

		int32 Removed = Group.PendingIds.RemoveAll([&](auto& Id) { return Id == TaskId; });
		ensure(Removed == 1);

		if (Group.PendingIds.Num() == 0)
		{
			RemoveGroup(*GroupWeakPtr, false);
		}
	}
}

void FVoxelTasksDependenciesHandler::RemoveGroup(TWeakPtr<FGroup> GroupWeakPtr, bool bTimeout)
{
	auto GroupPtr = GroupWeakPtr.Pin();
	if (GroupPtr.IsValid())
	{
		auto& Group = *GroupPtr;

		for (auto& Task : Group.Tasks)
		{
			TaskIdsToGroupsMap.Remove(Task.Id);
			auto ChunkPtr = Task.Chunk.Pin();
			if (ChunkPtr.IsValid())
			{
				ChunkPtr->WaitForDependenciesCallback(bTimeout);
			}
		}
		TaskIdsToGroupsMap.Compact();
		TasksGroups.Remove(GroupPtr);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelDefaultRenderer::FVoxelDefaultRenderer(const FVoxelRendererSettings& Settings)
	: IVoxelRenderer(Settings)
	, DependenciesHandler(Settings.World, Settings.WaitForOtherChunksToAvoidHolesTimeout)
{
}

FVoxelDefaultRenderer::~FVoxelDefaultRenderer()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelDefaultRenderer_FVoxelDefaultRenderer);
	for (auto& Chunk : ChunksMap)
	{
		// Destroy while we're still valid
		Chunk.Value->Destroy();
		ensure(Chunk.Value.GetSharedReferenceCount() == 1);
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::UpdateChunks(const TArray<uint64>& ChunksToUpdate, bool bWaitForAllChunksToFinishUpdating, const FVoxelOnUpdateFinished& FinishDelegate)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateChunks);
	
	if (ChunksToUpdate.Num() == 0)
	{
		// Always fire delegate
		FinishDelegate.ExecuteIfBound(FIntBox());
	}

	TArray<FIntBox> UpdatedChunks;

	TArray<FVoxelTasksDependenciesHandler::FLockedTask> Tasks;
	for (auto& ChunkId : ChunksToUpdate)
	{
		auto& Chunk = ChunksMap.FindChecked(ChunkId);
			
		uint64 Id = 0;
		if (bWaitForAllChunksToFinishUpdating && Chunk->CanStartUpdateWithCustomTaskId())
		{
			Id = DependenciesHandler.GetTaskId();
			Tasks.Emplace(Chunk, Id);
		}
		
		Chunk->UpdateChunk(Id, FinishDelegate);
		UpdatedChunks.Add(Chunk->Bounds);
	}
	DependenciesHandler.AddGroup(Tasks);

	Settings.DebugManager->ReportUpdatedChunks(UpdatedChunks);
}

void FVoxelDefaultRenderer::CancelDithering(const FIntBox& Bounds, const TArray<uint64>& Chunks)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_CancelDithering);

	for (auto& Chunk : ChunksToRemoveAfterDithering)
	{
		if (!Chunk->IsDone() && Chunk->GetBounds().Intersect(Bounds))
		{
			Chunk->Remove();
		}
	}
	ChunksToRemoveAfterDithering.RemoveAll([&](auto& Chunk) { return Chunk->IsDone(); });

	for (auto& ChunkId : Chunks)
	{
		ChunksMap.FindChecked(ChunkId)->CancelDithering();
	}
}

void FVoxelDefaultRenderer::RecomputeMeshPositions()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_RecomputeMeshPositions);

	for (auto& Chunk : ChunksMap)
	{
		Chunk.Value->RecomputeMeshPosition();
	}
	for (auto& Chunk : ChunksToRemoveAfterDithering)
	{
		Chunk->SetMeshRelativeLocation(Settings.GetChunkRelativePosition(Chunk->GetBounds().Min));
	}
}

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs ChunksToAdd"), STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToAdd, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs ChunksToUpdate1"), STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToUpdate1, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs ChunksToUpdate2"), STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToUpdate2, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs ChunksToRemove"), STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToRemove, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::UpdateLODs Update"), STAT_VoxelDefaultRenderer_UpdateLODs_Update, STATGROUP_Voxel);

void FVoxelDefaultRenderer::UpdateLODs(const TArray<FVoxelChunkToAdd>& ChunksToAdd, const TArray<FVoxelChunkToUpdate>& ChunksToUpdate, const TArray<FVoxelChunkToRemove>& ChunksToRemove, const TArray<FVoxelTransitionsToUpdate>& TransitionsToUpdate)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs);
	
	// For the first update (else the task count is 0 while the octree is building)
	bUpdatesStarted = true;

	TMap<uint64, TArray<FVoxelRenderChunk*>> OldChunksToNewChunks;
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToAdd);
		for (auto& ChunkToAdd : ChunksToAdd)
		{
			TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> NewChunk = GetRenderChunk(ChunkToAdd.LOD, ChunkToAdd.Bounds, ChunkToAdd.Settings);
			NewChunk->UpdateChunk(0);

			check(!ChunksMap.Contains(ChunkToAdd.Id));
			ChunksMap.Add(ChunkToAdd.Id, NewChunk);

			for (auto& PreviousChunkId : ChunkToAdd.PreviousChunks)
			{
				OldChunksToNewChunks.FindOrAdd(PreviousChunkId).Add(NewChunk.Get());
			}
		}
	}

	// First add chunks that are visible to OldChunksToNewChunks
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToUpdate1);
		for (auto& ChunkToUpdate : ChunksToUpdate)
		{
			if (ChunkToUpdate.NewSettings.bVisible)
			{
				auto* Chunk = ChunksMap.FindChecked(ChunkToUpdate.Id).Get();
				for (auto& PreviousChunkId : ChunkToUpdate.PreviousChunks)
				{
					ensure(!ChunkToUpdate.OldSettings.bVisible);
					OldChunksToNewChunks.FindOrAdd(PreviousChunkId).Add(Chunk);
				}
			}
		}
	}

	// Then add previous chunks
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToUpdate2);
		for (auto& ChunkToUpdate : ChunksToUpdate)
		{
			if (!ChunkToUpdate.NewSettings.bVisible)
			{
				auto& Chunk = ChunksMap.FindChecked(ChunkToUpdate.Id);
				if (ChunkToUpdate.OldSettings.bVisible && Chunk->NeedsToBeDeleted())
				{
					auto ChunkToDelete = MakeShared<FVoxelChunkToDelete>(*Chunk, true);
					if (auto* NewChunks = OldChunksToNewChunks.Find(ChunkToUpdate.Id))
					{
						for (auto* NewChunk : *NewChunks)
						{
							NewChunk->AddPreviousChunk(ChunkToDelete);
						}
					}
				}
			}
		}
	}

	// From ChunksToRemove too
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs_ChunksToRemove);
		for (auto& ChunkToRemove : ChunksToRemove)
		{
			TSharedPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> Chunk;
			verify(ChunksMap.RemoveAndCopyValue(ChunkToRemove.Id, Chunk));

			if (Chunk->NeedsToBeDeleted())
			{
				auto ChunkToDelete = MakeShared<FVoxelChunkToDelete>(*Chunk, false);
				if (auto* NewChunks = OldChunksToNewChunks.Find(ChunkToRemove.Id))
				{
					for (auto* NewChunk : *NewChunks)
					{
						NewChunk->AddPreviousChunk(ChunkToDelete);
					}
				}
			}
			
			Chunk->Destroy();
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_UpdateLODs_Update);
		// And only after all that update them
		for (auto& ChunkToUpdate : ChunksToUpdate)
		{
			auto& Chunk = ChunksMap.FindChecked(ChunkToUpdate.Id);
			ensure(Chunk->Settings == ChunkToUpdate.OldSettings);
			Chunk->UpdateSettings(ChunkToUpdate.NewSettings);
		}

		for (auto& TransitionToUpdate : TransitionsToUpdate)
		{
			auto& Chunk = ChunksMap.FindChecked(TransitionToUpdate.Id);
			Chunk->UpdateTransitions(TransitionToUpdate.TransitionsMask);
		}
	}

	Settings.DebugManager->ReportRenderChunks(ChunksMap);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::StartMeshDithering(UVoxelProceduralMeshComponent* Mesh, const FIntBox& Bounds, const TArray<TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>>& NewRenderChunks)
{	
	// 2x: first dither in new chunk, then dither out old chunk
	FVoxelRenderUtilities::StartMeshDithering(Mesh, 2 * Settings.ChunksDitheringDuration);
	auto Chunk = MakeUnique<FVoxelChunkToRemoveAfterDithering>(
		Mesh,
		Bounds,
		AsShared(),
		2 * Settings.ChunksDitheringDuration,
		NewRenderChunks
		);
	ChunksToRemoveAfterDithering.Add(MoveTemp(Chunk));
}

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::SetMeshPosition"), STAT_VoxelDefaultRenderer_SetMeshPosition, STATGROUP_Voxel);

void FVoxelDefaultRenderer::SetMeshPosition(UVoxelProceduralMeshComponent* Mesh, const FIntVector& Position)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_SetMeshPosition);
	Mesh->SetRelativeLocation(Settings.GetChunkRelativePosition(Position));
}

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::RemoveMesh"), STAT_VoxelDefaultRenderer_RemoveMesh, STATGROUP_Voxel);

void FVoxelDefaultRenderer::RemoveMesh(UVoxelProceduralMeshComponent* Mesh)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_RemoveMesh);

	Mesh->ClearSections();
	GetMeshPool(Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision).Add(Mesh);
}

DECLARE_CYCLE_STAT(TEXT("FVoxelDefaultRenderer::GetNewMesh"), STAT_VoxelDefaultRenderer_GetNewMesh, STATGROUP_Voxel);

UVoxelProceduralMeshComponent* FVoxelDefaultRenderer::GetNewMesh(const FIntVector& Position, uint8 LOD, bool bCollisions)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelDefaultRenderer_GetNewMesh);

	if (!Settings.ComponentsOwner.IsValid())
	{
		return nullptr;
	}

	UVoxelProceduralMeshComponent* NewMesh;
	auto& MeshPool = GetMeshPool(bCollisions);
	if (MeshPool.Num() > 0)
	{
		NewMesh = MeshPool.Pop();
	}
	else
	{
		NewMesh = NewObject<UVoxelProceduralMeshComponent>(Settings.ComponentsOwner.Get(), Settings.ProcMeshClass, NAME_None, RF_Transient);
		if (!bCollisions)
		{
			NewMesh->DisableCollisions();
		}
		NewMesh->Pool = Settings.Pool;
		NewMesh->SetupAttachment(Settings.ComponentsOwner->GetRootComponent(), NAME_None);
		NewMesh->RegisterComponent();
		NewMesh->SetRelativeScale3D(FVector::OneVector * Settings.VoxelSize);
		NewMesh->SetCollisionEnabled(bCollisions ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		if (bCollisions)
		{
			NewMesh->BodyInstance = Settings.CollisionPresets;
		}
	}

	SetMeshPosition(NewMesh, Position);

	if (Settings.World->WorldType != EWorldType::Editor)
	{
		NewMesh->InitChunk(LOD, FVoxelUtilities::GetBoundsFromPositionAndDepth<CHUNK_SIZE>(Position, LOD));
	}

	return NewMesh;
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::IncreaseTaskCount()
{
	TaskCount++;
	Settings.DebugManager->ReportTasksCount(TaskCount);
}

void FVoxelDefaultRenderer::DecreaseTaskCount()
{
	TaskCount--; 
	check(TaskCount >= 0);
	Settings.DebugManager->ReportTasksCount(TaskCount);

	if (TaskCount == 0 && !bOnLoadedCallbackFired)
	{
		OnWorldLoaded.Broadcast();
		bOnLoadedCallbackFired = true;
	}
}

uint64 FVoxelDefaultRenderer::GetSquaredDistanceToInvokers(const FIntVector& Position) const
{
	uint64 Distance = MAX_uint64;
	for (auto& InvokerPosition : *Settings.InvokersPositions)
	{
		Distance = FMath::Min<uint64>(Distance, FVoxelUtilities::SquaredSize(Position - InvokerPosition));
	}
	return Distance;
}

#undef LOCTEXT_NAMESPACE