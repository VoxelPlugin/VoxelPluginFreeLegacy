// Copyright 2020 Phyronnaz

#include "VoxelRendererBasicMeshHandler.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelChunkMaterials.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "IVoxelPool.h"
#include "VoxelAsyncWork.h"

#include "Async/Async.h"

class FVoxelBasicMeshMergeWork : public FVoxelAsyncWork
{
public:
	static FVoxelBasicMeshMergeWork* Create(
		FVoxelRendererBasicMeshHandler& Handler,
		FVoxelRendererBasicMeshHandler::FChunkInfoRef ChunkInfoRef,
		FVoxelChunkMeshesToBuild&& MeshesToBuild)
	{
		auto* ChunkInfo = Handler.GetChunkInfo(ChunkInfoRef);
		check(ChunkInfo);
		return new FVoxelBasicMeshMergeWork(
			ChunkInfoRef,
			ChunkInfo->Position,
			Handler,
			ChunkInfo->UpdateIndex.ToSharedRef(),
			MoveTemp(MeshesToBuild));
	}

private:
	const FVoxelRendererBasicMeshHandler::FChunkInfoRef ChunkInfoRef;
	const FIntVector Position;
	const FVoxelRendererSettingsBase RendererSettings;
	const TVoxelWeakPtr<FVoxelRendererBasicMeshHandler> Handler;

	const FVoxelChunkMeshesToBuild MeshesToBuild;
	const TVoxelSharedRef<FThreadSafeCounter> UpdateIndexPtr;
	const int32 UpdateIndex;

	FVoxelBasicMeshMergeWork(
		FVoxelRendererBasicMeshHandler::FChunkInfoRef Ref,
		const FIntVector& Position,
		FVoxelRendererBasicMeshHandler& Handler,
		const TVoxelSharedRef<FThreadSafeCounter>& UpdateIndexPtr,
		FVoxelChunkMeshesToBuild&& MeshesToBuild)
		: FVoxelAsyncWork(STATIC_FNAME("FVoxelBasicMeshMergeWork"), 1e9, true)
		, ChunkInfoRef(Ref)
		, Position(Position)
		, RendererSettings(static_cast<const FVoxelRendererSettingsBase&>(Handler.Renderer.Settings))
		, Handler(StaticCastVoxelSharedRef<FVoxelRendererBasicMeshHandler>(Handler.AsShared()))
		, MeshesToBuild(MoveTemp(MeshesToBuild))
		, UpdateIndexPtr(UpdateIndexPtr)
		, UpdateIndex(UpdateIndexPtr->GetValue())
	{
	}
	~FVoxelBasicMeshMergeWork() = default;

	virtual uint32 GetPriority() const override
	{
		return 0;
	}
	virtual void DoWork() override
	{
		if (UpdateIndexPtr->GetValue() > UpdateIndex)
		{
			// Canceled
			return;
		}
		auto BuiltMeshes = FVoxelRenderUtilities::BuildMeshes_AnyThread(MeshesToBuild, RendererSettings, Position, *UpdateIndexPtr, UpdateIndex);
		if (!BuiltMeshes.IsValid())
		{
			// Canceled
			return;
		}
		auto HandlerPinned = Handler.Pin();
		if (HandlerPinned.IsValid())
		{
			// Queue callback
			HandlerPinned->MeshMergeCallback(ChunkInfoRef, UpdateIndex, MoveTemp(BuiltMeshes));
			FVoxelUtilities::DeleteOnGameThread_AnyThread(HandlerPinned);
		}
	}
};

FVoxelRendererBasicMeshHandler::~FVoxelRendererBasicMeshHandler()
{
	FlushBuiltDataQueue();
	FlushActionQueue(MAX_dbl);
	
	ensure(ChunkInfos.Num() == 0);
}

IVoxelRendererMeshHandler::FChunkId FVoxelRendererBasicMeshHandler::AddChunkImpl(int32 LOD, const FIntVector& Position)
{
	return ChunkInfos.Add(FChunkInfo::Create(LOD, Position));
}

void FVoxelRendererBasicMeshHandler::ApplyAction(const FAction& Action)
{
	VOXEL_FUNCTION_COUNTER();
	
	switch (Action.Action)
	{
	case EAction::UpdateChunk:
	{
		check(Action.UpdateChunk().InitialCall.MainChunk);
		const auto& MainChunk = *Action.UpdateChunk().InitialCall.MainChunk;
		const auto* TransitionChunk = Action.UpdateChunk().InitialCall.TransitionChunk;

		// This should never happen, as the chunk should be removed instead
		ensure(!MainChunk.IsEmpty() || (TransitionChunk && !TransitionChunk->IsEmpty()));

		auto& ChunkInfo = ChunkInfos[Action.ChunkId];
		if (!ChunkInfo.Materials.IsValid())
		{
			ChunkInfo.Materials = MakeShared<FVoxelChunkMaterials>();
		}
		if (!ChunkInfo.UpdateIndex.IsValid())
		{
			ChunkInfo.UpdateIndex = MakeVoxelShared<FThreadSafeCounter>();
		}

		// Cancel any previous build task
		// Note: we do not clear the built data, as it could still be used
		// The added cost of applying the update is probably worth it compared to stalling the entire queue waiting for an update
		ChunkInfo.UpdateIndex->Increment();
			
		// Find the meshes to build (= copying mesh buffers to proc mesh buffers)
		FVoxelChunkMeshesToBuild MeshesToBuild = FVoxelRenderUtilities::GetMeshesToBuild(
			ChunkInfo.LOD,
			ChunkInfo.Position,
			Renderer.Settings,
			Action.UpdateChunk().InitialCall.ChunkSettings,
			*ChunkInfo.Materials,
			MainChunk,
			TransitionChunk,
			Renderer.OnMaterialInstanceCreated,
			ChunkInfo.DitheringInfo);

		// Start a task to asynchronously build them
		auto* Task = FVoxelBasicMeshMergeWork::Create(*this, { Action.ChunkId, ChunkInfo.UniqueId }, MoveTemp(MeshesToBuild));
		Renderer.Settings.Pool->QueueTask(EVoxelTaskType::MeshMerge, Task);

		FAction NewAction;
		NewAction.Action = EAction::UpdateChunk;
		NewAction.ChunkId = Action.ChunkId;
		NewAction.UpdateChunk().AfterCall.UpdateIndex = ChunkInfo.UpdateIndex->GetValue();
		NewAction.UpdateChunk().AfterCall.DistanceFieldVolumeData = Action.UpdateChunk().InitialCall.MainChunk->GetDistanceFieldVolumeData();
		ActionQueue.Enqueue(NewAction);
			
		if (Renderer.Settings.RenderType == EVoxelRenderType::SurfaceNets)
		{
			SetTransitionsMaskForSurfaceNets(Action.ChunkId, Action.UpdateChunk().InitialCall.ChunkSettings.TransitionsMask);
		}
		break;
	}
	case EAction::RemoveChunk:
	case EAction::DitherChunk:
	case EAction::ResetDithering:
	case EAction::SetTransitionsMaskForSurfaceNets:
	case EAction::HideChunk:
	case EAction::ShowChunk:
	{
		ActionQueue.Enqueue(Action);
		break;
	}
	default: ensure(false);
	}
}

void FVoxelRendererBasicMeshHandler::ClearChunkMaterials()
{
	for (auto& ChunkInfo : ChunkInfos)
	{
		if (ChunkInfo.Materials.IsValid())
		{
			ChunkInfo.Materials->Reset();
		}
	}
}

void FVoxelRendererBasicMeshHandler::Tick(double MaxTime)
{
	VOXEL_FUNCTION_COUNTER();

	IVoxelRendererMeshHandler::Tick(MaxTime);

	FlushBuiltDataQueue();
	FlushActionQueue(MaxTime);

	Renderer.Settings.DebugManager->ReportMeshActionQueueNum(ActionQueue.Num());
}

void FVoxelRendererBasicMeshHandler::FlushBuiltDataQueue()
{
	VOXEL_FUNCTION_COUNTER();

	// Copy built data from async task callbacks to the chunk infos
	// Should be fast enough to not require checking the time
	FBuildCallback Callback;
	while (CallbackQueue.Dequeue(Callback))
	{
		auto& BuiltData = Callback.BuiltData;

		if (!ensure(BuiltData.BuiltMeshes.IsValid())) continue;

		auto* ChunkInfo = GetChunkInfo(Callback.ChunkInfoRef);
		if (ChunkInfo && BuiltData.UpdateIndex >= ChunkInfo->UpdateIndex->GetValue())
		{
			// Not outdated
			ensure(BuiltData.UpdateIndex == ChunkInfo->UpdateIndex->GetValue());
			ChunkInfo->BuiltData = MoveTemp(BuiltData);
		}
	}
}

void FVoxelRendererBasicMeshHandler::FlushActionQueue(double MaxTime)
{
	VOXEL_FUNCTION_COUNTER();
	
	FAction Action;
	// Peek: if UpdateChunk isn't ready yet we don't want to pop the action
	// Always process dithering in immediately, as else the chunk will be showed until the next tick and then hidden (one frame glitch)
	while (ActionQueue.Peek(Action) && 
		(FPlatformTime::Seconds() < MaxTime ||
		 (Action.Action == EAction::DitherChunk && Action.DitherChunk().DitheringType == EDitheringType::Classic_DitherIn)))
	{
		auto& ChunkInfo = ChunkInfos[Action.ChunkId];
		CleanUp(ChunkInfo.Meshes);

		if (IsDestroying() && Action.Action != EAction::RemoveChunk)
		{
			ActionQueue.Pop();
			continue;
		}

		switch (Action.Action)
		{
		case EAction::UpdateChunk:
		{
			const int32 WantedUpdateIndex = Action.UpdateChunk().AfterCall.UpdateIndex;
			if (ChunkInfo.MeshUpdateIndex >= WantedUpdateIndex)
			{
				// Already updated
				// This happens when a previous UpdateChunk used the built data we triggered
				break;
			}
			if (WantedUpdateIndex > ChunkInfo.BuiltData.UpdateIndex)
			{
				// Not built yet, wait

				if (ChunkInfo.BuiltData.UpdateIndex != -1)
				{
					// Stored built data is outdated, clear it to save memory
					ensure(ChunkInfo.BuiltData.BuiltMeshes.IsValid());
					ChunkInfo.BuiltData.BuiltMeshes.Reset();
					ChunkInfo.BuiltData.UpdateIndex = -1;
				}

				return;
			}

			// Move to clear the built data value
			const auto BuiltMeshes = MoveTemp(ChunkInfo.BuiltData.BuiltMeshes);
			ChunkInfo.MeshUpdateIndex = ChunkInfo.BuiltData.UpdateIndex;
			ChunkInfo.BuiltData.UpdateIndex = -1;

			if (!ensure(BuiltMeshes.IsValid())) continue;

			int32 MeshIndex = 0;
			// Apply built meshes
			for (auto& BuiltMesh : *BuiltMeshes)
			{
				const FVoxelMeshConfig& MeshConfig = BuiltMesh.Key;
				if (ChunkInfo.Meshes.Num() <= MeshIndex)
				{
					// Not enough meshes to render the built mesh, allocate new ones
					auto* NewMesh = GetNewMesh(Action.ChunkId, ChunkInfo.Position, ChunkInfo.LOD);
					if (!ensureVoxelSlow(NewMesh)) return;
					ChunkInfo.Meshes.Add(NewMesh);
				}

				auto& Mesh = *ChunkInfo.Meshes[MeshIndex];
				MeshConfig.ApplyTo(Mesh);

				Mesh.SetDistanceFieldData(nullptr);
				Mesh.ClearSections(EVoxelProcMeshSectionUpdate::DelayUpdate);
				for (auto& Section : BuiltMesh.Value)
				{
					if (!ensure(Section.Value.IsValid())) continue;
					Mesh.AddProcMeshSection(Section.Key, MoveTemp(Section.Value), EVoxelProcMeshSectionUpdate::DelayUpdate);
				}
				Mesh.FinishSectionsUpdates();

				MeshIndex++;
			}

			// Clear unused meshes
			for (; MeshIndex < ChunkInfo.Meshes.Num(); MeshIndex++)
			{
				auto& Mesh = *ChunkInfo.Meshes[MeshIndex];
				Mesh.SetDistanceFieldData(nullptr);
				Mesh.ClearSections(EVoxelProcMeshSectionUpdate::UpdateNow);
			}

			// Handle distance fields
			const auto& DistanceFieldVolumeData = Action.UpdateChunk().AfterCall.DistanceFieldVolumeData;
			if (DistanceFieldVolumeData.IsValid())
			{
				// Use the first mesh to hold the distance field data
				// We should always have at least one mesh, else the chunk should have been removed instead of updated
				if (ensure(ChunkInfo.Meshes.Num() > 0))
				{
					ChunkInfo.Meshes[0]->SetDistanceFieldData(DistanceFieldVolumeData);
				}
			}
			break;
		}
		case EAction::RemoveChunk:
		{
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				RemoveMesh(*Mesh);
			}
			ChunkInfos.RemoveAt(Action.ChunkId);
			break;
		}
		case EAction::DitherChunk:
		{
			ChunkInfo.DitheringInfo.bIsValid = true;
			ChunkInfo.DitheringInfo.DitheringType = Action.DitherChunk().DitheringType;
			ChunkInfo.DitheringInfo.Time = FVoxelRenderUtilities::GetWorldCurrentTime(Renderer.Settings.World.Get());
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				FVoxelRenderUtilities::StartMeshDithering(
					*Mesh,
					Renderer.Settings,
					ChunkInfo.DitheringInfo);
			}
			break;
		}
		case EAction::ResetDithering:
		{
			ChunkInfo.DitheringInfo.bIsValid = false;
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				FVoxelRenderUtilities::ResetDithering(*Mesh, Renderer.Settings);
			}
			break;
		}
		case EAction::SetTransitionsMaskForSurfaceNets:
		{
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				FVoxelRenderUtilities::SetMeshTransitionsMask(*Mesh, Action.SetTransitionsMaskForSurfaceNets().TransitionsMask);
			}
			break;
		}
		case EAction::HideChunk:
		{
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				FVoxelRenderUtilities::HideMesh(*Mesh);
			}
			break;
		}
		case EAction::ShowChunk:
		{
			for (auto& Mesh : ChunkInfo.Meshes)
			{
				FVoxelRenderUtilities::ShowMesh(*Mesh);
			}
			break;
		}
		default: ensure(false);
		}

		if (CVarLogActionQueue.GetValueOnGameThread() != 0)
		{
			LOG_VOXEL(Log, TEXT("ActionQueue: LOD: %d; %s; Position: %s"), ChunkInfo.LOD, *Action.ToString(), *ChunkInfo.Position.ToString());
		}

		ActionQueue.Pop();
	}
}

void FVoxelRendererBasicMeshHandler::MeshMergeCallback(FChunkInfoRef ChunkInfoRef, int32 UpdateIndex, TUniquePtr<FVoxelBuiltChunkMeshes> BuiltMeshes)
{
	CallbackQueue.Enqueue({ ChunkInfoRef, FChunkBuiltData{ UpdateIndex,  MoveTemp(BuiltMeshes) } });
}