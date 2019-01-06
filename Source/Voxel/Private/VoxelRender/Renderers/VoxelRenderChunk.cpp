// Copyright 2019 Phyronnaz

#include "VoxelRenderChunk.h"
#include "VoxelLODRenderer.h"
#include "VoxelWorld.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelPoolManager.h"
#include "VoxelRender/VoxelRenderUtilities.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::EndTasks"), STAT_VoxelRenderChunk_EndTasks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::Tick"), STAT_VoxelRenderChunk_Tick, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::Update"), STAT_VoxelRenderChunk_Update, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderChunk::UpdateMeshFromMainChunk"), STAT_VoxelRenderChunk_UpdateMeshFromMainChunk, STATGROUP_Voxel);

FVoxelRenderChunk::FVoxelRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds)
	: Render(Render)
	, Position(Bounds.Min)
	, LOD(LOD)
	, Bounds(Bounds)
	, ChunkMaterials(MakeShared<FVoxelChunkMaterials>(&Render->World->MaterialsRef))
{
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::Destroy()
{
	bDestroyed = true;
	// Release previous chunks, as else they have an invalid ref to Render
	PreviousChunks.Reset();
	EndTasks();
}

void FVoxelRenderChunk::AddPreviousChunk(const TSharedRef<FVoxelChunkToDelete>& Chunk)
{
	PreviousChunks.Add(Chunk);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::ResetAlpha()
{
	if (Mesh)
	{
		for (int Index = 0; Index < Mesh->GetNumSections(); Index++)
		{
			UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Mesh->GetProcMeshSection(Index)->Material);
			if (Material)
			{
				Material->SetScalarParameterValue(FName(TEXT("StartTime")), 0);
			}
		}
	}
}

void FVoxelRenderChunk::RecreateMaterials()
{
	ChunkMaterials->Reset();
	if (Mesh)
	{
		UpdateMeshFromChunks();
	}
}

void FVoxelRenderChunk::RecomputeMeshPosition()
{
	if (Mesh)
	{
		Mesh->SetRelativeLocation(Render->World->GetChunkRelativePosition(Position));
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::SetScalarParameterValue(FName ParameterName, float Value)
{
	ChunkMaterials->SetScalarParameterValue(ParameterName, Value);
}

void FVoxelRenderChunk::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
	ChunkMaterials->SetTextureParameterValue(ParameterName, Value);
}

void FVoxelRenderChunk::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
	ChunkMaterials->SetVectorParameterValue(ParameterName, Value);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::UpdateChunk(uint64 InTaskId)
{
	QueuedTaskId = InTaskId;
	if (MeshTask.IsValid())
	{
		bNeedUpdate = true;
		ScheduleTick();
	}
	else
	{
		Update();
	}
}

void FVoxelRenderChunk::UpdateTransitions(uint8 NewTransitionsMask)
{
	if (WantedTransitionsMask != NewTransitionsMask)
	{
		WantedTransitionsMask = NewTransitionsMask;
		ScheduleTick();
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::Tick()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_Tick);

	if (bNeedUpdate)
	{
		if (MeshTask.IsValid())
		{
			ScheduleTick();
		}
		else
		{
			Update();
		}
	}

	if (MeshTask.IsValid())
	{
		if (MeshTask->IsDone() && Render->CanApplyTask(TaskId))
		{
			MainChunk = MeshTask.Get()->Chunk;

			UpdateMeshFromChunks();


			Render->DecreaseTaskCount();
			MeshTask.Reset();
			PreviousChunks.Reset(); // Free previous chunks
		}
	}

	bool bTransitionsMeshNeedUpdate = false;

	if (TransitionsTask.IsValid() && TransitionsTask->IsDone())
	{
		check(TransitionsMaskBeingComputed == TransitionsTask->TransitionsMask);

		TransitionChunk = TransitionsTask->Chunk;

		TransitionsDisplayedMask = TransitionsMaskBeingComputed;
		bTransitionsMeshNeedUpdate = true;
		
		Render->DecreaseTaskCount();
		TransitionsTask.Reset();
	}

	if (TransitionsDisplayedMask != WantedTransitionsMask || bTransitionsNeedUpdate)
	{
		if (TransitionsTask.IsValid() && Render->RetractQueuedWork(TransitionsTask.Get()))
		{
			TransitionsTask.Reset();
			Render->DecreaseTaskCount();
		}
		// Will queue if retract failed, or do it now if succeeded
		UpdateTransitions();
	}

	if (MeshDisplayedMask != TransitionsDisplayedMask || bTransitionsMeshNeedUpdate)
	{
		UpdateMeshFromChunks();
	}
}

void FVoxelRenderChunk::ScheduleTick()
{
	Render->ScheduleTick(this);
}

bool FVoxelRenderChunk::IsDone() const
{
	return !MeshTask || MeshTask->IsDone();
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::UpdateMeshFromChunks()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_UpdateMeshFromMainChunk);
	
	if (!Mesh && MainChunk.IsValid() && (!MainChunk->IsEmpty() || (TransitionChunk.IsValid() && !TransitionChunk->IsEmpty())))
	{
		Mesh = Render->GetNewMesh(Position, LOD);
	}

	if (Mesh)
	{
		FVoxelRenderUtilities::CreateMeshSectionFromChunks(
			LOD,
			IsInitialLoad(),
			Render->World,
			Mesh,
			ChunkMaterials,
			MainChunk,
			Render->World->GetRenderType() == EVoxelRenderType::MarchingCubes ? TransitionsDisplayedMask : 0,
			TransitionChunk);

		MeshDisplayedMask = TransitionsDisplayedMask;
	}
}

void FVoxelRenderChunk::EndTasks()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_EndTasks);

	if (MeshTask.IsValid())
	{
		if (!MeshTask->IsDone() && !Render->RetractQueuedWork(MeshTask.Get()))
		{
			MeshTask->CancelAndAutodelete();
			MeshTask.Release();
		}
		Render->DecreaseTaskCount();
	}

	if (TransitionsTask.IsValid())
	{
		if (!TransitionsTask->IsDone() && !Render->RetractQueuedWork(TransitionsTask.Get()))
		{
			TransitionsTask->CancelAndAutodelete();
			TransitionsTask.Release();
		}
		Render->DecreaseTaskCount();
	}
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelRenderChunk::Update()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelRenderChunk_Update);

	check(!MeshTask.IsValid());

	MeshTask = GetNewTask(PreviousGrassInfo);

	Render->AddQueuedWork(MeshTask.Get());
	Render->IncreaseTaskCount();

	bNeedUpdate = false;
	TaskId = QueuedTaskId;
	QueuedTaskId = 0;

	UpdateTransitions();
}

void FVoxelRenderChunk::UpdateTransitions()
{
	if (!TransitionsTask.IsValid())
	{
		if (WantedTransitionsMask != 0)
		{
			TransitionsTask = GetNewTransitionTask(WantedTransitionsMask);
			Render->AddQueuedWork(TransitionsTask.Get());
			Render->IncreaseTaskCount();
			TransitionsMaskBeingComputed = WantedTransitionsMask;
		}
		else
		{			
			TransitionChunk.Reset();
			TransitionsDisplayedMask = WantedTransitionsMask;
		}
		bTransitionsNeedUpdate = false;
	}
	else
	{
		bTransitionsNeedUpdate = true;
	}
}

