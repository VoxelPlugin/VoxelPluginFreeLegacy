// Copyright 2019 Phyronnaz

#include "VoxelRenderChunk.h"
#include "VoxelDefaultRenderer.h"
#include "VoxelWorld.h"
#include "IVoxelPool.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

#include "Async/Async.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::Destroy"), STAT_VoxelRenderChunk_Destroy, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::EndTasks"), STAT_VoxelRenderChunk_EndTasks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::UpdateSettings"), STAT_VoxelRenderChunk_UpdateSettings, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("BuildTessellation"), STAT_VoxelRenderChunk_BuildAdjacency, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::UpdateFromMeshTask"), STAT_VoxelRenderChunk_UpdateFromMeshTask, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::UpdateFromTransitionsTask"), STAT_VoxelRenderChunk_UpdateFromTransitionsTask, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::StartUpdate"), STAT_VoxelRenderChunk_StartUpdate, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::StartTransitionsUpdate"), STAT_VoxelRenderChunk_StartTransitionsUpdate, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::UpdateMeshFromMainChunk"), STAT_VoxelRenderChunk_UpdateMeshFromMainChunk, STATGROUP_Voxel);

DECLARE_MEMORY_STAT(TEXT("Voxel Render Chunks Memory"), STAT_VoxelRenderChunksMemory, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Render Chunks Count"), STAT_VoxelRenderChunksCount, STATGROUP_VoxelMemory);

FVoxelRenderChunk::FVoxelRenderChunk(FVoxelDefaultRenderer* Renderer, uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& Settings)
	: Renderer(Renderer)
	, Position(Bounds.Min)
	, LOD(LOD)
	, Bounds(Bounds)
	, Settings(Settings)
{
	INC_DWORD_STAT_BY(STAT_VoxelRenderChunksCount, 1);
	INC_MEMORY_STAT_BY(STAT_VoxelRenderChunksMemory, sizeof(FVoxelRenderChunk));
}

FVoxelRenderChunk::~FVoxelRenderChunk()
{
	DEC_DWORD_STAT_BY(STAT_VoxelRenderChunksCount, 1);
	DEC_MEMORY_STAT_BY(STAT_VoxelRenderChunksMemory, sizeof(FVoxelRenderChunk));

	ensure(bDestroyed);
	ensure(OnUpdate.Num() == 0);
	ensure(OnQueuedUpdate.Num() == 0);
	ensure(!MeshTask.IsValid());
	ensure(!TransitionsTask.IsValid());
	ensure(!ChunkMaterials.IsValid());
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::Destroy()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_Destroy);

	bDestroyed = true;
	// Release previous chunks, as else they have an invalid ref to Render
	PreviousChunks.Reset();
	ChunkMaterials.Reset();
	EndTasks();
}

void FVoxelRenderChunk::EndTasks()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_EndTasks);

	if (MeshTask.IsValid())
	{
		MeshTask->CancelAndAutodelete();
		MeshTask.Release();
		Renderer->DecreaseTaskCount();
	}

	if (TransitionsTask.IsValid())
	{
		TransitionsTask->CancelAndAutodelete();
		TransitionsTask.Release();
		Renderer->DecreaseTaskCount();
	}

	// Always fire delegates
	for (auto& Delegate : OnUpdate)
	{
		Delegate.ExecuteIfBound(Bounds);
	}
	for (auto& Delegate : OnQueuedUpdate)
	{
		Delegate.ExecuteIfBound(Bounds);
	}
	OnUpdate.Reset();
	OnQueuedUpdate.Reset();
}

void FVoxelRenderChunk::AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk)
{
	PreviousChunks.Add(Chunk);
	Chunk->AddNewChunk(AsShared());
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::CancelDithering()
{
	auto& TimerManager = Renderer->Settings.World->GetTimerManager();
	TimerManager.ClearTimer(HideMeshAfterDitheringHandle);

	if (Mesh)
	{
		if (Settings.bVisible || FVoxelRenderUtilities::DebugInvisibleChunks())
		{
			FVoxelRenderUtilities::ResetDithering(Mesh);
		}
		else
		{
			FVoxelRenderUtilities::HideMesh(Mesh);
		}
	}
}

void FVoxelRenderChunk::RecomputeMeshPosition()
{
	if (Mesh)
	{
		Renderer->SetMeshPosition(Mesh, Position);
	}
}

void FVoxelRenderChunk::UpdateSettings(const FVoxelRenderChunkSettings& NewSettings)
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_UpdateSettings);

	ensure(Settings != NewSettings);
	FVoxelRenderChunkSettings OldSettings = Settings;
	Settings = NewSettings;

	if (OldSettings.bEnableCollisions != NewSettings.bEnableCollisions && Mesh)
	{
		Renderer->RemoveMesh(Mesh);
		Mesh = nullptr;
	}
	if (NewSettings.bVisible && !OldSettings.bVisible)
	{
		CancelDithering();
	}
	if (OldSettings.bEnableTessellation != NewSettings.bEnableTessellation)
	{
		if (ChunkMaterials.IsValid())
		{
			ChunkMaterials->Reset();
		}
	}

	if (!NewSettings.bVisible && OldSettings.bVisible)
	{
		// Hack to allow dithering instead of hiding the section
		Settings.bVisible = true;
	}
	UpdateMeshFromChunks();
	if (!NewSettings.bVisible && OldSettings.bVisible)
	{
		Settings.bVisible = false;
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::UpdateChunk(uint64 InTaskId, const FVoxelOnUpdateFinished& Delegate)
{
	OnQueuedUpdate.Add(Delegate);
	if (MeshTask.IsValid() && !TryToUpdateFromMeshTask())
	{
		check(InTaskId == 0);
		bUpdateQueued = true;
	}
	else
	{
		TaskId = InTaskId;
		StartUpdate();
	}
}

void FVoxelRenderChunk::UpdateTransitions(uint8 NewTransitionsMask)
{
	if (WantedTransitionsMask != NewTransitionsMask)
	{
		WantedTransitionsMask = NewTransitionsMask;
		UpdateTransitions();
	}
}

void FVoxelRenderChunk::UpdateTransitions()
{
	if (TransitionsTask.IsValid() && !TryToUpdateFromTransitionsTask())
	{
		bTransitionsUpdateQueued = true;
	}
	else
	{
		StartTransitionsUpdate();
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::MeshCallback(uint64 InTaskId)
{
	Renderer->ReportTaskFinished(InTaskId);
	if (MeshTask.IsValid())
	{
		TryToUpdateFromMeshTask();
	}
	if (bUpdateQueued && !MeshTask.IsValid())
	{
		StartUpdate();
	}
}

void FVoxelRenderChunk::TransitionsCallback()
{
	if (TransitionsTask.IsValid())
	{
		TryToUpdateFromTransitionsTask();
	}
	if (bTransitionsUpdateQueued && !TransitionsTask.IsValid())
	{
		StartTransitionsUpdate();
	}
}

void FVoxelRenderChunk::WaitForDependenciesCallback(bool bTimeout)
{
	ensure(MeshTask.IsValid());

	bool bSuccess = TryToUpdateFromMeshTask();
	ensure(bSuccess || bTimeout);

	if (bUpdateQueued && bSuccess && ensure(!MeshTask.IsValid()))
	{
		StartUpdate();
	}
}

void FVoxelRenderChunk::NewChunksAreUpdated()
{
	if (!Settings.bVisible && Mesh) // If it's still needed
	{
		if (FVoxelRenderUtilities::DebugInvisibleChunks())
		{
			UpdateMeshFromChunks();
		}
		else
		{
			auto& RendererSettings = Renderer->Settings;
			auto& TimerManager = RendererSettings.World->GetTimerManager();
			
			TimerManager.SetTimer(HideMeshAfterDitheringHandle, FTimerDelegate::CreateThreadSafeSP(this, &FVoxelRenderChunk::HideMeshAfterDithering), RendererSettings.GetRealChunksDitheringDuration(), false);
			FVoxelRenderUtilities::StartMeshDithering(Mesh, RendererSettings);
		}
	}
}

void FVoxelRenderChunk::HideMeshAfterDithering()
{
	if (!Settings.bVisible && Mesh && !FVoxelRenderUtilities::DebugInvisibleChunks()) // If it's still needed
	{
		FVoxelRenderUtilities::HideMesh(Mesh);
	}
}

bool FVoxelRenderChunk::CanStartUpdateWithCustomTaskId() const
{
	return !MeshTask.IsValid() || (MeshTask->IsDone() && Renderer->CanApplyTask(TaskId));
}

uint64 FVoxelRenderChunk::GetPriority() const
{
	return MAX_uint64 - Renderer->GetSquaredDistanceToInvokers(Position);
}

///////////////////////////////////////////////////////////////////////////////

bool FVoxelRenderChunk::TryToUpdateFromMeshTask()
{
	check(MeshTask.IsValid());

	if (MeshTask->IsDone() && Renderer->CanApplyTask(TaskId))
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_UpdateFromMeshTask);

		MainChunk = MeshTask.Get()->Chunk;

		Renderer->DecreaseTaskCount();
		MeshTask.Reset();

		UpdateMeshFromChunks();

		for (auto& Delegate : OnUpdate)
		{
			Delegate.ExecuteIfBound(Bounds);
		}
		OnUpdate.Reset();

		return true;
	}
	else
	{
		return false;
	}
}

bool FVoxelRenderChunk::TryToUpdateFromTransitionsTask()
{
	check(TransitionsTask.IsValid());

	if (TransitionsTask->IsDone())
	{
		SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_UpdateFromTransitionsTask);

		check(TransitionsBeingComputedMask == TransitionsTask->TransitionsMask);

		TransitionsChunk = TransitionsTask->Chunk;

		TransitionsChunkMask = TransitionsBeingComputedMask;

		Renderer->DecreaseTaskCount();
		TransitionsTask.Reset();

		UpdateMeshFromChunks();

		return true;
	}
	else
	{
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::UpdateMeshFromChunks()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_UpdateMeshFromMainChunk);

	if (!Mesh && MainChunk.IsValid() && (!MainChunk->IsEmpty() || (TransitionsChunk.IsValid() && !TransitionsChunk->IsEmpty())))
	{
		Mesh = Renderer->GetNewMesh(Position, LOD, Settings.bEnableCollisions);
	}

	if (Mesh)
	{
		if (!ChunkMaterials)
		{
			ChunkMaterials = MakeUnique<FVoxelChunkMaterials>();
		}
		
		if (Settings.bEnableTessellation)
		{
			SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_BuildAdjacency);
			auto BuildAdjacency = [](auto& Buffer)
			{
				if (!Buffer.HasAdjacency())
				{
					Buffer.BuildAdjacency();
				}
			};
			if (MainChunk.IsValid())
			{
				MainChunk->IterateOnBuffers(BuildAdjacency);
			}
			if (TransitionsChunk.IsValid())
			{
				TransitionsChunk->IterateOnBuffers(BuildAdjacency);
			}
		}

		FVoxelRenderUtilities::CreateMeshSectionFromChunks(
			LOD,
			GetPriority(),
			IsInitialLoad(),
			Renderer->Settings,
			Settings,
			Mesh,
			*ChunkMaterials,
			MainChunk,
			TransitionsChunkMask,
			TransitionsChunk);

		MeshDisplayedMask = TransitionsChunkMask;
	}

	if (MainChunk.IsValid())
	{
		PreviousChunks.Reset(); // Free previous chunks AFTER updating as IsInitialLoad uses them, and only if the update was really done
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::StartUpdate()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_StartUpdate);

	check(!MeshTask.IsValid());

	MeshTask = GetNewTask();

	Renderer->Settings.Pool->QueueMeshingTask(MeshTask.Get());
	Renderer->IncreaseTaskCount();

	bUpdateQueued = false;
	check(OnUpdate.Num() == 0);
	Swap(OnUpdate, OnQueuedUpdate);

	UpdateTransitions();
}

void FVoxelRenderChunk::StartTransitionsUpdate()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_StartTransitionsUpdate);

	check(!TransitionsTask.IsValid());

	if (WantedTransitionsMask != 0)
	{
		TransitionsTask = GetNewTransitionTask();
		Renderer->Settings.Pool->QueueMeshingTask(TransitionsTask.Get());
		Renderer->IncreaseTaskCount();
		TransitionsBeingComputedMask = WantedTransitionsMask;
	}
	else
	{
		TransitionsChunk.Reset();
		TransitionsChunkMask = WantedTransitionsMask;
		UpdateMeshFromChunks();
	}
	bTransitionsUpdateQueued = false;
}