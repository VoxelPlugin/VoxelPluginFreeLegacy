// Copyright 2020 Phyronnaz

#include "VoxelDefaultRenderer.h"
#include "VoxelMessages.h"
#include "IVoxelPool.h"
#include "VoxelRender/VoxelMesherAsyncWork.h"
#include "VoxelRender/Renderers/VoxelRendererMeshHandler.h"
#include "VoxelRender/Renderers/VoxelRendererBasicMeshHandler.h"
#include "VoxelRender/Renderers/VoxelRendererClusteredMeshHandler.h"
#include "VoxelRender/Renderers/VoxelRendererMixedMeshHandler.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelRenderer);

static TAutoConsoleVariable<int32> CVarFreezeRenderer(
	TEXT("voxel.renderer.FreezeRenderer"),
	0,
	TEXT("Stops renderer tick"),
	ECVF_Default);

FVoxelDefaultRenderer::FVoxelDefaultRenderer(const FVoxelRendererSettings& Settings)
	: IVoxelRenderer(Settings)
	, MeshHandler(Settings.bMergeChunks ? Settings.bDoNotMergeCollisionsAndNavmesh
		? StaticCastVoxelSharedRef<IVoxelRendererMeshHandler>(MakeVoxelShared<FVoxelRendererMixedMeshHandler>(*this))
		: StaticCastVoxelSharedRef<IVoxelRendererMeshHandler>(MakeVoxelShared<FVoxelRendererClusteredMeshHandler>(*this))
		: StaticCastVoxelSharedRef<IVoxelRendererMeshHandler>(MakeVoxelShared<FVoxelRendererBasicMeshHandler>(*this)))
{
	MeshHandler->Init();
}

TVoxelSharedRef<FVoxelDefaultRenderer> FVoxelDefaultRenderer::Create(const FVoxelRendererSettings& Settings)
{
	TVoxelSharedRef<FVoxelDefaultRenderer> Renderer = MakeShareable(new FVoxelDefaultRenderer(Settings));
	Renderer->OnMaterialInstanceCreated.AddThreadSafeSP(Settings.Data->Generator, &FVoxelGeneratorInstance::SetupMaterialInstance);
	return Renderer;
}

FVoxelDefaultRenderer::~FVoxelDefaultRenderer()
{
	check(IsInGameThread());
	VOXEL_FUNCTION_COUNTER();

	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderer, AllocatedSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::Destroy()
{
	// This function is needed because the async tasks can keep the renderer alive while the voxel world is destroyed
	
	StopTicking();

	// Destroy mesh handler & meshes
	MeshHandler->StartDestroying();

	for (auto& It : ChunksMap)
	{
		CancelTasks(It.Value);
		if (It.Value.MeshId.IsValid())
		{
			// Not really needed, but useful for error checks
			MeshHandler->RemoveChunk(It.Value.MeshId);
		}
	}

	ChunksMap.Reset();
	MeshHandler.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelDefaultRenderer::UpdateChunks(
	const FVoxelIntBox& Bounds,
	const TArray<uint64>& ChunksToUpdate, 
	const FVoxelOnChunkUpdateFinished& FinishDelegate)
{
	VOXEL_FUNCTION_COUNTER();

	if (Settings.bStaticWorld)
	{
		FVoxelMessages::Error("Can't update chunks with bStaticWorld = true!");
		return 0;
	}
	
	if (ChunksToUpdate.Num() == 0)
	{
		return 0;
	}
	
	const double Time = FPlatformTime::Seconds();
	for (auto& ChunkId : ChunksToUpdate)
	{
		auto& Chunk = ChunksMap.FindChecked(ChunkId);
		Chunk.PendingUpdates.Add({ Time, FinishDelegate });
		// Trigger tasks if not already triggered: if they are, they will trigger new ones when their callback will be processed in Tick
		StartTask<EMainOrTransitions::Main, EIfTaskExists::DoNothing>(Chunk);
		StartTask<EMainOrTransitions::Transitions, EIfTaskExists::DoNothing>(Chunk);
	}
	
	FlushQueuedTasks();

	if (Settings.bDitherChunks)
	{
		VOXEL_SCOPE_COUNTER("Cancel Dithering");

		FVoxelIntBoxWithValidity ChunksToRemoveBounds;
		// First remove all chunks that are dithering out
		for (auto& ChunkToRemove : ChunksToRemove)
		{
			FChunk& Chunk = ChunksMap.FindChecked(ChunkToRemove.Id);
			if (Chunk.Bounds.Intersect(Bounds))
			{
				ChunkToRemove.Time = 0;
				ChunksToRemoveBounds += Chunk.Bounds;
			}
		}

		// Next force show all chunks dithering in overlapping the chunks dithering out we removed
		// Else they will be holes
		// This also covers chunks dithering in overlapping Bounds:
		// if they are dithering in, some other chunk overlapping these same bounds must be dithering out
		// This chunk will have been picked up in the iteration above
		if (ChunksToRemoveBounds.IsValid())
		{
			for (auto& ChunkToShow : ChunksToShow)
			{
				FChunk& Chunk = ChunksMap.FindChecked(ChunkToShow.Id);
				if (Chunk.Bounds.Intersect(ChunksToRemoveBounds.GetBox()))
				{
					ChunkToShow.Time = 0;
				}
			}
		}

		// Force update as we don't want to have any outdated chunks that could be used if UpdateLODs is called before Tick
		if (ChunksToRemoveBounds.IsValid())
		{
			// If invalid, no chunk was removed nor shown
			ProcessChunksToRemoveOrShow();
		}
	}

	Settings.DebugManager->ReportUpdatedChunks([&]()
		{
			TArray<FVoxelIntBox> UpdatedChunks;
			UpdatedChunks.Reserve(ChunksToUpdate.Num());
			for (auto& ChunkId : ChunksToUpdate)
			{
				auto& Chunk = ChunksMap.FindChecked(ChunkId);
				UpdatedChunks.Add(Chunk.Bounds);
			}
			return UpdatedChunks;
		});

	return ChunksToUpdate.Num();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::UpdateLODs(const uint64 InUpdateIndex, const TArray<FVoxelChunkUpdate>& ChunkUpdates)
{
	VOXEL_FUNCTION_COUNTER();

	check(InUpdateIndex > 0);
	if (Settings.bStaticWorld && InUpdateIndex != 1)
	{
		FVoxelMessages::Error("Can't update LODs with bStaticWorld = true!");\
		return;
	}

#if VOXEL_DEBUG
	{
		TSet<uint64> Ids;
		for (auto& ChunkUpdate : ChunkUpdates)
		{
			ensure(!Ids.Contains(ChunkUpdate.Id));
			Ids.Add(ChunkUpdate.Id);
		}
	}
	for (auto& ChunkUpdate : ChunkUpdates)
	{
		if (auto* DebugChunk = DebugChunks.Find(ChunkUpdate.Id))
		{
			ensure(*DebugChunk == ChunkUpdate.OldSettings);
			if (ChunkUpdate.NewSettings.HasRenderChunk())
			{
				*DebugChunk = ChunkUpdate.NewSettings;
			}
			else
			{
				DebugChunks.Remove(ChunkUpdate.Id);
			}
		}
		else
		{
			ensure(!ChunkUpdate.OldSettings.HasRenderChunk());
			DebugChunks.Add(ChunkUpdate.Id, ChunkUpdate.NewSettings);
		}
	}
#endif
	
	UpdateIndex++;
	if (!ensure(UpdateIndex == InUpdateIndex)) return;

	// Map used to know which chunks to wait for before dithering out
	TMap<uint64, TArray<uint64, TInlineAllocator<8>>> OldChunksToNewChunks;
	// Need to do it after the main pass, else OldChunksToNewChunks wouldn't be filled
	TArray<uint64> ChunksToDitherOutOrRemoveOnceNewChunksAreUpdated;
	// Can't call ClearPreviousChunks in the Update loop as old chunks are not processed yet
	TArray<uint64> ChunksPendingClearPreviousChunks;
	
	for (auto& ChunkUpdate : ChunkUpdates)
	{
		const auto NewSettings = ChunkUpdate.NewSettings;
		const auto OldSettings = ChunkUpdate.OldSettings;

		const auto GetChunk = [&]() -> FChunk&
		{
			FChunk* Chunk = ChunksMap.Find(ChunkUpdate.Id);
			if (Chunk)
			{
				ensure(Chunk->LOD == ChunkUpdate.LOD && Chunk->Bounds == ChunkUpdate.Bounds);
			}
			else
			{
				ensure(!OldSettings.HasRenderChunk());
				Chunk = &ChunksMap.Add(ChunkUpdate.Id, FChunk(ChunkUpdate.Id, ChunkUpdate.LOD, ChunkUpdate.Bounds));
			}
			check(Chunk);
			return *Chunk;
		};

		FChunk& Chunk = GetChunk();
		// Can only have pending settings if dithering out (force visible = true) or waiting for new chunks (force visible = true, force collisions/navmesh)
		ensure(
			Chunk.Settings == Chunk.PendingSettings ||
			Chunk.GetState() == EChunkState::DitheringOut ||
			Chunk.GetState() == EChunkState::WaitingForNewChunks);
		ensure(Chunk.PendingSettings == OldSettings);

		// Take a backup of the actual chunk settings
		// These will be different than OldSettings if DitheringOut or WaitingForNewChunks
		const auto OldChunksSettings = Chunk.Settings;
		
		// We set the chunk settings here so that we have the right priority when starting tasks below
		Chunk.Settings = NewSettings;

		// By default, apply the visibility & transitions change
		bool bApplyNewVisibilityAndMask = true;

		// Check if visibility changed (most common case)
		if (NewSettings.bVisible != OldSettings.bVisible)
		{
			const auto CancelDithering = [&]()
			{
				VOXEL_SCOPE_COUNTER("CancelDithering");
				
				ensure(Chunk.GetState() == EChunkState::DitheringIn || Chunk.GetState() == EChunkState::DitheringOut);
				ensure(Settings.bDitherChunks);
				ensure(Chunk.NumNewChunksLeft == 0);
				if (Chunk.MeshId.IsValid())
				{
					MeshHandler->ResetDithering(Chunk.MeshId);
				}
				if (Chunk.GetState() == EChunkState::DitheringOut)
				{
					ensure(Chunk.PreviousChunks.Num() == 0);
					ChunksToRemove.RemoveAllSwap([&](auto& X) { return X.Id == Chunk.Id; }, false);
				}
				if (Chunk.GetState() == EChunkState::DitheringIn)
				{
					ensure(Settings.bDitherChunks);
					// Note: will probably have previous chunks
					ChunksToShow.RemoveAllSwap([&](auto& X) { return X.Id == Chunk.Id; }, false);
				}
			};
			
			if (NewSettings.bVisible)
			{
				switch (Chunk.GetState())
				{
				case EChunkState::Showed:
				default:
				{
					ensure(false);
					break;
				}
				case EChunkState::WaitingForNewChunks:
				{
					ensure(Chunk.NumNewChunksLeft != 0);
					Chunk.NumNewChunksLeft = 0;
					ensure(Chunk.Settings.HasRenderChunk() || (!Chunk.Tasks.MainTask.IsValid() && !Chunk.Tasks.TransitionsTask.IsValid()));
					if (Chunk.BuiltData.MainChunk.IsValid())
					{
						// Do not clear previous chunks now as it's not safe to do so while in Update (as old chunks have not been processed now)
						// This chunk is already updated, no need to wait for it
						ChunksPendingClearPreviousChunks.Add(Chunk.Id);
					}
					else
					{
						// We got put in the WaitingForNewChunks state without a finished task
						// Start it now
						StartTask<EMainOrTransitions::Main, EIfTaskExists::Assert>(Chunk);
					}
					break;
				}
				case EChunkState::DitheringOut:
				{
					ensure(Chunk.MeshId.IsValid()); // Chunks that are dithering out always have a mesh
					ensure(Chunk.NumNewChunksLeft == 0);
					ensure(Chunk.PreviousChunks.Num() == 0);
					CancelDithering();
						
					// Tricky detail: when dithering out, we're not in the render octree anymore
					// This means we're not updated when an edit happens
					// However, we're still safe because editing cancels dithering!
					// So if there was an edit, the chunk would already be deleted
						
					// This chunk is already updated, no need to wait for it
					ChunksPendingClearPreviousChunks.Add(Chunk.Id);
					break;
				}
				case EChunkState::DitheringIn:
				{
					ensure(false); // Cannot happen as it would mean we were dithering in with bVisible = false
					ensure(Chunk.NumNewChunksLeft == 0);
					CancelDithering();
					if (!Chunk.Tasks.MainTask.IsValid() && !Chunk.Tasks.TransitionsTask.IsValid())
					{
						// If we have not tasks but still dithering in, then the mesh must be already updated
						// No need to wait
						ChunksPendingClearPreviousChunks.Add(Chunk.Id);
					}
					break;
				}
				case EChunkState::Hidden:
				{
					ensure(Chunk.NumNewChunksLeft == 0);
					ensure(Chunk.PreviousChunks.Num() == 0);
					if (Chunk.MeshId.IsValid())
					{
						// Note: will be shown by ApplyPendingSettings below
						if (Settings.bDitherChunks)
						{
							// Dither as we were hidden
							// Dither out of the other chunks will happen when processing ChunksPendingClearPreviousChunks
							// So both surface nets cases are correctly handled
							DitherInChunk(Chunk, ChunkUpdate.PreviousChunks);
						}
					}
					// This chunk is already updated, no need to wait for it
					ChunksPendingClearPreviousChunks.Add(Chunk.Id);
					break;
				}
				case EChunkState::NewChunk:
				{
					StartTask<EMainOrTransitions::Main, EIfTaskExists::Assert>(Chunk);
					break;
				}
				}
				ensure(Chunk.NumNewChunksLeft == 0);

				// Set UpdateIndex as we are being showed
				Chunk.UpdateIndex = UpdateIndex;
				
				Chunk.SetState(Settings.bDitherChunks ? EChunkState::DitheringIn : EChunkState::Showed, STATIC_FNAME("Turned visible"));
				if (!Chunk.MeshId.IsValid())
				{
					// If we don't have a mesh:
					// - either we started a task that will update it later on
					// - or a task has already finished & the resulting chunk was empty
					ensure(
						Chunk.Tasks.MainTask.IsValid() ||
						(Chunk.BuiltData.MainChunk.IsValid() && Chunk.BuiltData.MainChunk->IsEmpty()));
				}

				// When updating if we don't have a mesh yet we always need previous chunks, even with no dithering because we need to wait for task to be done
				// If we did have a mesh then we added ourselves to ChunksPendingClearPreviousChunks in the switch above
				// In both cases we can safely add the previous chunks
				for (auto& PreviousChunkId : ChunkUpdate.PreviousChunks)
				{
					OldChunksToNewChunks.FindOrAdd(PreviousChunkId).Add(ChunkUpdate.Id);
				}
			}
			else // !NewSettings.bVisible
			{
				switch (Chunk.GetState())
				{
				case EChunkState::NewChunk:
				case EChunkState::Hidden:
				default:
				{
					ensure(false);
					break;
				}
				case EChunkState::WaitingForNewChunks:
				{
					ensure(Chunk.NumNewChunksLeft != 0);
					ensure(Chunk.MeshId.IsValid() || Chunk.PreviousChunks.Num() != 0); // Either we had a mesh or chunks to wait for
					// Keep previous chunks along, will be cleared when this will be cleared
					break;
				}
				case EChunkState::DitheringOut:
				{
					ensure(false); // Cannot happen as it would mean we were dithering out with bVisible = true
					ensure(Chunk.NumNewChunksLeft == 0);
					ensure(Chunk.PreviousChunks.Num() == 0);
					CancelDithering();
					break;
				}
				case EChunkState::DitheringIn:
				{
					ensure(Chunk.NumNewChunksLeft == 0);
					// Note: might have previous chunks, keep them
					CancelDithering();
					break;
				}
				case EChunkState::Showed:
				{
					// Note: can have previous chunks if main chunk finished dithering but transitions are still being computed
					break;
				}
				}

				// We can't be in any of these states if we get here
				ensureVoxelSlowNoSideEffects(!ChunksToRemove.FindByPredicate([&](const FChunkToRemove& ChunkToRemove) { return ChunkToRemove.Id == Chunk.Id; }));
				ensureVoxelSlowNoSideEffects(!ChunksToShow.FindByPredicate([&](const FChunkToShow& ChunkToShow) { return ChunkToShow.Id == Chunk.Id; }));
				
				if (!NewSettings.HasRenderChunk())
				{
					// If this chunk is being removed, no need to finish computing the tasks
					CancelTasks(Chunk);
				}

				if (!Chunk.MeshId.IsValid() && Chunk.PreviousChunks.Num() == 0)
				{
					ensure(Chunk.NumNewChunksLeft == 0);
					if (!NewSettings.HasRenderChunk())
					{
						// No mesh nor previous chunks: remove it immediately
						DestroyChunk(Chunk);
						continue;
					}
					else
					{
						// No mesh nor chunks to wait for: can just set the state to hidden
						Chunk.SetState(EChunkState::Hidden, STATIC_FNAME(""));
					}
				}
				else
				{
					Chunk.SetState(EChunkState::WaitingForNewChunks, STATIC_FNAME("Hiding chunk"));
					ChunksToDitherOutOrRemoveOnceNewChunksAreUpdated.Add(Chunk.Id);
				}
			}
			
			// Can never happen after an update
			// Might lead to some abrupt changes, but w/e it's pretty bad already if we get there
			ensure(Chunk.GetState() != EChunkState::DitheringOut);
		}
		else
		{
			// Other case: visibility did not change. This is an update to the state of collisions/navmesh
			// Much less frequent
			switch (Chunk.GetState())
			{
			case EChunkState::NewChunk:
			{
				// Hidden new chunks
				ensure(!NewSettings.bVisible);
				ensure(NewSettings.HasRenderChunk() && !OldSettings.HasRenderChunk());
				StartTask<EMainOrTransitions::Main, EIfTaskExists::Assert>(Chunk);
				Chunk.SetState(EChunkState::Hidden, STATIC_FNAME("Hidden New Chunk"));
				break;
			}
			case EChunkState::Hidden:
			{
				// If we were hidden and we still are, something else changed.
				// Check that we are still rendered
				if (!NewSettings.HasRenderChunk())
				{
					// If not remove the mesh if valid, and destroy
					// Make sure to cancel any pending tasks first
					CancelTasks(Chunk);
					if (Chunk.MeshId.IsValid())
					{
						MeshHandler->RemoveChunk(Chunk.MeshId);
						Chunk.MeshId.Reset();
					}
					DestroyChunk(Chunk);
					continue;
				}
				// Else just stay hidden, ApplyPendingSettings below will do the job
				break;
			}
			case EChunkState::DitheringIn:
			{
				// ApplyPendingSettings will do the job
				ensure(NewSettings.bVisible);
				break;
			}
			case EChunkState::Showed:
			{
				// ApplyPendingSettings will do the job
				ensure(NewSettings.bVisible);
				break;
			}
			case EChunkState::WaitingForNewChunks:
			{
				// ApplyPendingSettings will do the job
				// Make sure we don't update visibility/transitions though
				bApplyNewVisibilityAndMask = false;
				// Should be invisible for the LOD tree, but visible here
				ensure(!NewSettings.bVisible);
				ensure(OldChunksSettings.bVisible);
				break;
			}
			case EChunkState::DitheringOut:
			{
				// ApplyPendingSettings will do the job
				// Make sure we don't update visibility/transitions though
				bApplyNewVisibilityAndMask = false;
				// Should be invisible for the LOD tree, but visible here
				ensure(!NewSettings.bVisible);
				ensure(OldChunksSettings.bVisible);
				break;
			}
			default: ensure(false);
			}
		}
		
		// Settings were changed for priority, set them back
		// Make sure to use OldChunksSettings and not OldSettings
		Chunk.Settings = OldChunksSettings;
		// Set new settings
		Chunk.PendingSettings = NewSettings;

		if (Chunk.GetState() == EChunkState::WaitingForNewChunks)
		{
			// We don't want to hide it just yet if it's not visible anymore, as we need to dither it out/wait for new chunks to be updated
			// Same for collisions/navmesh, we don't want things to fall through while new chunks are still loading
			// So skip ApplyPendingSettings
			continue;
		}

		// Finally, apply all the new settings
		// Only apply new visibility if it actually changed
		// This is to keep a chunk that's dithering out or waiting for new chunks visible, even if for the LOD tree it's not
		// They will correctly call ApplyPendingSettings once updated
		ApplyPendingSettings(Chunk, bApplyNewVisibilityAndMask);

		// Else stack is cleared when debugging
		ensureVoxelSlowNoSideEffects(&Chunk);
	}
	
	FlushQueuedTasks();
	
	{
		VOXEL_SCOPE_COUNTER("Old Chunks");
		// Now that OldChunksToNewChunks is fully built, add previous chunks
		for (uint64 OldChunkId : ChunksToDitherOutOrRemoveOnceNewChunksAreUpdated)
		{
			auto* NewChunks = OldChunksToNewChunks.Find(OldChunkId);

			// NewChunks is invalid when we don't have new chunks to replace us
			// This seems to only happen when bEnableRender is set to false at runtime
			
			FChunk& OldChunk = ChunksMap.FindChecked(OldChunkId);
			ensure(OldChunk.MeshId.IsValid() || OldChunk.PreviousChunks.Num() > 0); // We need to have a mesh or be waiting for previous ones
			ensure(OldChunk.NumNewChunksLeft == 0);
			ensure(OldChunk.GetState() == EChunkState::WaitingForNewChunks);

			if (NewChunks)
			{
				for (uint64 NewChunkId : *NewChunks)
				{
					auto& NewChunk = ChunksMap.FindChecked(NewChunkId);
					// AddUnique: in some cases NewChunks is already referencing us
					NewChunk.PreviousChunks.AddUnique(OldChunkId);
					OldChunk.NumNewChunksLeft++;
				}
			}
			
			// Might need to remove it/dither it out now if no new chunks
			if (OldChunk.NumNewChunksLeft == 0)
			{
				ClearPreviousChunks(OldChunk);
				RemoveOrHideChunk(OldChunk);
			}
		}
	}

	{
		VOXEL_SCOPE_COUNTER("ClearPreviousChunks");
		// Process all meshes already displayed
		// Need to do it because needs to be done after old chunks
		for (uint64 Id : ChunksPendingClearPreviousChunks)
		{
			ClearPreviousChunks(ChunksMap.FindChecked(Id));
		}
	}

	Settings.DebugManager->ReportRenderChunks([&]()
		{
			TArray<FVoxelIntBox> Result;
			Result.Reserve(ChunksMap.Num());
			for (auto& It : ChunksMap)
			{
				Result.Add(It.Value.Bounds);
			}
			return Result;
		});

	FlushQueuedTasks();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 FVoxelDefaultRenderer::GetTaskCount() const
{
	return UpdateIndex > 0 ? TaskCount.GetValue() : -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::RecomputeMeshPositions()
{
	VOXEL_FUNCTION_COUNTER();

	MeshHandler->RecomputeMeshPositions();
}

void FVoxelDefaultRenderer::ApplyNewMaterials()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (Settings.bStaticWorld)
	{
		FVoxelMessages::Error("Can't ApplyNewMaterials with bStaticWorld = true!");
		return;
	}

	Settings.OnMaterialsChanged();

	MeshHandler->ClearChunkMaterials();

	for (auto& It : ChunksMap)
	{
		const auto& Chunk = It.Value;
		if (Chunk.MeshId.IsValid() && ensure(Chunk.BuiltData.MainChunk.IsValid()))
		{
			MeshHandler->UpdateChunk(
				Chunk.MeshId,
				Chunk.Settings,
				*Chunk.BuiltData.MainChunk,
				Chunk.BuiltData.TransitionsChunk.Get(),
				Chunk.BuiltData.TransitionsMask);
		}
	}
}

void FVoxelDefaultRenderer::ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	MeshHandler->ApplyToAllMeshes(Lambda);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::CreateGeometry_AnyThread(
	int32 LOD,
	const FIntVector& ChunkPosition,
	TArray<uint32>& OutIndices,
	TArray<FVector>& OutVertices) const
{
	FVoxelMesherAsyncWork::CreateGeometry_AnyThread(*this, LOD, ChunkPosition, OutIndices, OutVertices);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDefaultRenderer::Tick(float)
{
	VOXEL_FUNCTION_COUNTER();

	if (CVarFreezeRenderer.GetValueOnGameThread() != 0)
	{
		return;
	}

	const double Time = FPlatformTime::Seconds();
	const double MaxTime = Time + Settings.MeshUpdatesBudget * 0.001f;
	
	{
		VOXEL_SCOPE_COUNTER("MeshHandler Tick");
		MeshHandler->Tick(MaxTime);
	}
	
	ProcessChunksToRemoveOrShow();
	ProcessMeshUpdates(MaxTime);
	FlushQueuedTasks();

	if (!OnWorldLoadedFired && UpdateIndex > 0 && TaskCount.GetValue() == 0 && TasksCallbacksQueue.IsEmpty())
	{
		OnWorldLoaded.Broadcast();
		OnWorldLoadedFired = true;
	}

	UpdateAllocatedSize();
	
	Settings.DebugManager->ReportMeshTaskCount(TaskCount.GetValue());
	Settings.DebugManager->ReportMeshTasksCallbacksQueueNum(TasksCallbacksQueue.Num());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<FVoxelDefaultRenderer::EMainOrTransitions MainOrTransitions, FVoxelDefaultRenderer::EIfTaskExists IfTaskExists>
void FVoxelDefaultRenderer::StartTask(FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();
	
	auto& Task = MainOrTransitions == EMainOrTransitions::Main ? Chunk.Tasks.MainTask : Chunk.Tasks.TransitionsTask;
	if (Task.IsValid())
	{
		if (IfTaskExists == EIfTaskExists::DoNothing)
		{
			return;
		}
		else
		{
			ensure(IfTaskExists == EIfTaskExists::Assert);
			ensure(false);
			return;
		}
	}

	if (MainOrTransitions == EMainOrTransitions::Transitions)
	{
		if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
		{
			if (Chunk.MeshId.IsValid())
			{
				MeshHandler->SetTransitionsMaskForSurfaceNets(Chunk.MeshId, Chunk.Settings.TransitionsMask);
			}
			Chunk.BuiltData.TransitionsChunkCreationTime = FPlatformTime::Seconds(); // Make sure to always update the time
			return;
		}
		if (Chunk.Settings.TransitionsMask == 0)
		{
			if (Chunk.BuiltData.TransitionsChunk.IsValid() && !Chunk.BuiltData.TransitionsChunk->IsEmpty())
			{
				// Remove transitions
				Chunk.BuiltData.TransitionsChunk.Reset();
				if (Chunk.MeshId.IsValid())
				{
					// If we have a main chunk, use it. Else remove the mesh entirely.
					if (Chunk.BuiltData.MainChunk.IsValid() && !Chunk.BuiltData.MainChunk->IsEmpty())
					{
						MeshHandler->UpdateChunk(Chunk.MeshId, Chunk.Settings, *Chunk.BuiltData.MainChunk, nullptr, 0);
					}
					else
					{
						MeshHandler->RemoveChunk(Chunk.MeshId);
						Chunk.MeshId.Reset();
					}
				}
			}
			Chunk.BuiltData.TransitionsMask = 0;
			Chunk.BuiltData.TransitionsChunkCreationTime = FPlatformTime::Seconds(); // Make sure to always update the time
			return;
		}
	}

	Task = TUniquePtr<FVoxelMesherAsyncWork, TVoxelAsyncWorkDelete<FVoxelMesherAsyncWork>>(new FVoxelMesherAsyncWork(
		*this,
		Chunk.Id,
		Chunk.LOD,
		Chunk.Bounds,
		MainOrTransitions == EMainOrTransitions::Transitions,
		MainOrTransitions == EMainOrTransitions::Transitions ? Chunk.Settings.TransitionsMask : 0));
	QueuedTasks[Chunk.Settings.bVisible][Chunk.Settings.bEnableCollisions].Emplace(Task.Get());
}

void FVoxelDefaultRenderer::CancelTasks(FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();

	if (Chunk.Tasks.MainTask.IsValid())
	{
		CancelTask(Chunk.Tasks.MainTask);
	}
	if (Chunk.Tasks.TransitionsTask.IsValid())
	{
		CancelTask(Chunk.Tasks.TransitionsTask);
	}
}

void FVoxelDefaultRenderer::ClearPreviousChunks(FChunk& Chunk)
{
	if (Chunk.PreviousChunks.Num() == 0) return;
	
	VOXEL_FUNCTION_COUNTER();
	
	static TSet<uint64> StackIds;
	if (!ensure(!StackIds.Contains(Chunk.Id))) return;
	StackIds.Add(Chunk.Id);
	
	for (uint64 PreviousChunkId : Chunk.PreviousChunks)
	{
		FChunk* PreviousChunkPtr = ChunksMap.Find(PreviousChunkId);
		if (!ensure(PreviousChunkPtr)) continue; // This crashed on Prod
		
		FChunk& PreviousChunk = *PreviousChunkPtr;
		if (PreviousChunk.UpdateIndex > Chunk.UpdateIndex) continue; // This previous chunk has been updated since it was added to our PreviousChunk list
		
		ensure(PreviousChunk.GetState() == EChunkState::WaitingForNewChunks); // Should be true if the UpdateIndex is correct

		PreviousChunk.NumNewChunksLeft--;
		if (!ensure(PreviousChunk.NumNewChunksLeft >= 0)) continue;

		if (PreviousChunk.NumNewChunksLeft == 0)
		{
			 // If 0 and have no previous chunks shouldn't be a previous chunk and should already be deleted
			if (!ensure(PreviousChunk.MeshId.IsValid() || PreviousChunk.PreviousChunks.Num() > 0)) continue;
			NewChunksFinished(PreviousChunk, Chunk);
		}
	}
	
	Chunk.PreviousChunks.Reset();

	ensure(StackIds.Remove(Chunk.Id) == 1);
}

void FVoxelDefaultRenderer::NewChunksFinished(FChunk& Chunk, const FChunk& NewChunk)
{
	VOXEL_FUNCTION_COUNTER();

	// Only visible chunks need to wait
	ensure(Chunk.Settings.bVisible);
	
	ensure(Chunk.GetState() == EChunkState::WaitingForNewChunks);
	ensure(Chunk.NumNewChunksLeft == 0);
	ensureVoxelSlowNoSideEffects(!ChunksToRemove.FindByPredicate([&](const FChunkToRemove& ChunkToRemove) { return ChunkToRemove.Id == Chunk.Id; }));
	ensureVoxelSlowNoSideEffects(!ChunksToShow.FindByPredicate([&](const FChunkToShow& ChunkToShow) { return ChunkToShow.Id == Chunk.Id; }));
	
	ClearPreviousChunks(Chunk); // Recursively delete previous chunks

	if (Settings.bDitherChunks && Chunk.MeshId.IsValid()) // Could be 0 if we were waiting for previous chunks
	{
		if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
		{
			// For surface nets, the only chunk that can transition is the high res one
			// So check if we're the high res one, and if not just delete self
			if (NewChunk.LOD > Chunk.LOD)
			{
				ApplyPendingSettings(Chunk, false);
				ensure(Chunk.MeshId.IsValid());
				ensure(Chunk.Settings.bVisible);
				
				Chunk.SetState(EChunkState::DitheringOut, STATIC_FNAME("NewChunksFinished"));
				MeshHandler->DitherChunk(Chunk.MeshId, EDitheringType::SurfaceNets_HighResToLowRes);
				ChunksToRemove.Add(FChunkToRemove{ Chunk.Id, FPlatformTime::Seconds() + Settings.ChunksDitheringDuration });
			}
			else
			{
				RemoveOrHideChunk(Chunk);
			}
		}
		else
		{
			ApplyPendingSettings(Chunk, false);
			ensure(Chunk.MeshId.IsValid());
			ensure(Chunk.Settings.bVisible);

			Chunk.SetState(EChunkState::DitheringOut, STATIC_FNAME("NewChunksFinished"));
			MeshHandler->DitherChunk(Chunk.MeshId, EDitheringType::Classic_DitherOut);
			// 2x: First dithering in new chunk, then dither out old chunk
			ChunksToRemove.Add(FChunkToRemove{ Chunk.Id, FPlatformTime::Seconds() + 2 * Settings.ChunksDitheringDuration });
		}
	}
	else
	{
		RemoveOrHideChunk(Chunk);
	}
}

void FVoxelDefaultRenderer::RemoveOrHideChunk(FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();
	
	ensure(Chunk.PreviousChunks.Num() == 0);
	ensure(Chunk.NumNewChunksLeft == 0);
	
	// DitheringOut if dithering enabled, else it's removed once WaitingForNewChunks is over
	ensure(Chunk.GetState() == EChunkState::DitheringOut || Chunk.GetState() == EChunkState::WaitingForNewChunks);
	
	ensureVoxelSlowNoSideEffects(!ChunksToRemove.FindByPredicate([&](const FChunkToRemove& ChunkToRemove) { return ChunkToRemove.Id == Chunk.Id; }));
	ensureVoxelSlowNoSideEffects(!ChunksToShow.FindByPredicate([&](const FChunkToShow& ChunkToShow) { return ChunkToShow.Id == Chunk.Id; }));

	// Note: MeshId might be 0 if we were waiting for other chunks
	
	if (Chunk.GetState() == EChunkState::DitheringOut)
	{
		ensure(Chunk.MeshId.IsValid()); // If we were dithering out, we must have a mesh
		// Reset the dithering to be safe when showing this mesh again
		MeshHandler->ResetDithering(Chunk.MeshId);
	}

	// Make sure to check PendingSettings and not Settings
	if (Chunk.PendingSettings.HasRenderChunk())
	{
		ApplyPendingSettings(Chunk, true); // Can apply the real settings now
		ensure(Chunk.Settings == Chunk.PendingSettings);
		Chunk.SetState(EChunkState::Hidden, STATIC_FNAME("RemoveOrHideChunk"));
	}
	else
	{
		CancelTasks(Chunk);
		if (Chunk.MeshId.IsValid())
		{
			MeshHandler->RemoveChunk(Chunk.MeshId);
			Chunk.MeshId.Reset();
		}
		DestroyChunk(Chunk);
		// Chunk is removed, no need to apply pending settings
	}
}

void FVoxelDefaultRenderer::DitherInChunk(FChunk& Chunk, const TArray<uint64, TInlineAllocator<8>>& PreviousChunks)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!ensure(Chunk.MeshId.IsValid())) return;
	
	ensureVoxelSlowNoSideEffects(!ChunksToRemove.FindByPredicate([&](const FChunkToRemove& ChunkToRemove) { return ChunkToRemove.Id == Chunk.Id; }));
	ensureVoxelSlowNoSideEffects(!ChunksToShow.FindByPredicate([&](const FChunkToShow& ChunkToShow) { return ChunkToShow.Id == Chunk.Id; }));
	
	if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
	{
		// For surface nets, the only chunk that can transition is the high res one
		// So check if we're the high res one, and if not just hide self until previous one finished dithering

		// If no previous chunk nothing to transition from, just show
		if (PreviousChunks.Num() > 0)
		{
			const FChunk& PreviousChunk = ChunksMap[PreviousChunks[0]];
			// PreviousChunk is always valid, as the LOD of a specific Id is always the same
			// No need to check UpdateIndex etc

			if (PreviousChunk.LOD > Chunk.LOD)
			{
				// We are the high res one
				MeshHandler->DitherChunk(Chunk.MeshId, EDitheringType::SurfaceNets_LowResToHighRes);
				ChunksToShow.Add(FChunkToShow{ Chunk.Id, FPlatformTime::Seconds() + Settings.ChunksDitheringDuration });
			}
			else
			{
				// We are the low res: the high res will do the work
				// Note: bTransitionsChunkIsBuilt is always true for surface nets, so dithering will happen at the same time for both
				MeshHandler->HideChunk(Chunk.MeshId);
				ChunksToShow.Add(FChunkToShow{ Chunk.Id, FPlatformTime::Seconds() + Settings.ChunksDitheringDuration });
			}
		}
	}
	else
	{
		MeshHandler->DitherChunk(Chunk.MeshId, EDitheringType::Classic_DitherIn);
		ChunksToShow.Add(FChunkToShow{ Chunk.Id, FPlatformTime::Seconds() + Settings.ChunksDitheringDuration });
	}
}

void FVoxelDefaultRenderer::ApplyPendingSettings(FChunk& Chunk, bool bApplyVisibility)
{
	VOXEL_FUNCTION_COUNTER();

	const bool bInitialHasMesh_Debug = Chunk.MeshId.IsValid();
	
	const auto OldSettings = Chunk.Settings;
	auto NewSettings = Chunk.PendingSettings;

	if (!bApplyVisibility)
	{
		NewSettings.bVisible = OldSettings.bVisible;
		// Make sure the TransitionsMask stays the same as we do not want to update transitions
		NewSettings.TransitionsMask = OldSettings.TransitionsMask;
	}

	// Only these two states are allowed to have different settings
	ensure(
		Chunk.GetState() == EChunkState::DitheringOut ||
		Chunk.GetState() == EChunkState::WaitingForNewChunks ||
		NewSettings == Chunk.PendingSettings);

	// ApplyPendingSettings does not handle removing a chunk
	ensure(NewSettings.HasRenderChunk());
	
	Chunk.Settings = NewSettings;

	if (NewSettings.bVisible            != OldSettings.bVisible          ||
		NewSettings.bEnableCollisions   != OldSettings.bEnableCollisions ||
		NewSettings.bEnableNavmesh      != OldSettings.bEnableNavmesh)
	{
		if (Chunk.MeshId.IsValid() && ensure(Chunk.BuiltData.MainChunk.IsValid())) // If we have a mesh we must have a built chunk
		{
			MeshHandler->UpdateChunk(
				Chunk.MeshId,
				Chunk.Settings,
				*Chunk.BuiltData.MainChunk,
				Chunk.BuiltData.TransitionsChunk.Get(),
				Chunk.BuiltData.TransitionsMask);
		}
	}

	// Important: do not update transitions if bApplyVisibility = false
	// Note: this might not be needed since we copying OldSettings.TransitionsMask to NewSettings.TransitionsMask, but do it anyways
	// as it would be a waste of CPU time to compute new transitions for a chunk dithering out
	if (bApplyVisibility)
	{
		// Check transitions now
		const uint8 WantedMask = NewSettings.TransitionsMask;
		if (Chunk.Tasks.TransitionsTask.IsValid())
		{
			if (Chunk.Tasks.TransitionsTask->TransitionsMask != WantedMask)
			{
				// Task already started and has different mask, cancel it
				CancelTask(Chunk.Tasks.TransitionsTask);
				if (Chunk.BuiltData.TransitionsMask != WantedMask)
				{
					// Start new task only if mask changed
					StartTask<EMainOrTransitions::Transitions, EIfTaskExists::Assert>(Chunk);
				}
				else
				{
					// Could be that this task was triggered by an update
					// If so we might need to start a new task even if the mask is the same
					CheckPendingUpdates(Chunk);
				}
			}
		}
		else 
		{
			if (Chunk.BuiltData.TransitionsMask != WantedMask)
			{
				// Mask changed, start new task
				StartTask<EMainOrTransitions::Transitions, EIfTaskExists::Assert>(Chunk);
			}
		}
	}

	// if bApplyVisibility = false, we must not change the MeshId
	ensure(bApplyVisibility || bInitialHasMesh_Debug == Chunk.MeshId.IsValid());
}

void FVoxelDefaultRenderer::CheckPendingUpdates(FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();
	
	for (int32 Index = 0; Index < Chunk.PendingUpdates.Num(); Index++)
	{
		const auto& PendingUpdate = Chunk.PendingUpdates[Index];
		if (PendingUpdate.WantedUpdateTime > Chunk.BuiltData.MainChunkCreationTime)
		{
			StartTask<EMainOrTransitions::Main, EIfTaskExists::DoNothing>(Chunk);
		}
		if (PendingUpdate.WantedUpdateTime > Chunk.BuiltData.TransitionsChunkCreationTime)
		{
			StartTask<EMainOrTransitions::Transitions, EIfTaskExists::DoNothing>(Chunk);
		}
		if (PendingUpdate.WantedUpdateTime < FMath::Min(Chunk.BuiltData.MainChunkCreationTime, Chunk.BuiltData.TransitionsChunkCreationTime))
		{
			PendingUpdate.OnUpdateFinished.Broadcast(Chunk.Bounds);
			Chunk.PendingUpdates.RemoveAtSwap(Index);
			Index--;
		}
	}
}

void FVoxelDefaultRenderer::ProcessChunksToRemoveOrShow()
{
	VOXEL_FUNCTION_COUNTER();
	
	const double Time = FPlatformTime::Seconds();

	// Process ChunksToShow before ChunksToRemove for action queue ordering
	
	{
		VOXEL_SCOPE_COUNTER("Processing ChunksToShow");
		ensure(Settings.bDitherChunks || ChunksToShow.Num() == 0);
		for (int32 Index = 0; Index < ChunksToShow.Num(); Index++)
		{
			const auto ChunkToShow = ChunksToShow[Index];
			if (ChunkToShow.Time < Time)
			{
				FChunk& Chunk = ChunksMap.FindChecked(ChunkToShow.Id);
				if (Chunk.GetState() != EChunkState::DitheringIn) continue; // Chunk is not dithering in anymore

				// ensure(Chunk.PreviousChunks.Num() == 0); Not always true: main chunk can have finished dithering but transitions still being computed
				Chunk.SetState(EChunkState::Showed, STATIC_FNAME("ChunkToShow"));

				if (Chunk.MeshId.IsValid())
				{
					if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
					{
						// If we were the low res chunk we were hidden
						MeshHandler->ShowChunk(Chunk.MeshId);
					}
					else
					{
						// Needed if it was canceled in UpdateChunks
						MeshHandler->ResetDithering(Chunk.MeshId);
					}
				}

				ChunksToShow.RemoveAtSwap(Index, 1, false);
				Index--; // Go back to process the element we swapped
			}
		}
	}

	{
		VOXEL_SCOPE_COUNTER("Processing ChunksToRemove");
		ensure(Settings.bDitherChunks || ChunksToRemove.Num() == 0);
		for (int32 Index = 0; Index < ChunksToRemove.Num(); Index++)
		{
			const auto ChunkToRemove = ChunksToRemove[Index];
			if (ChunkToRemove.Time < Time)
			{
				FChunk& Chunk = ChunksMap.FindChecked(ChunkToRemove.Id);
				if (Chunk.GetState() != EChunkState::DitheringOut) continue; // Chunk is not dithering out anymore

				ChunksToRemove.RemoveAtSwap(Index, 1, false);
				Index--; // Go back to process the element we swapped
				
				// Do it after so that it's not in ChunksToRemove anymore
				// for the checks to pass
				RemoveOrHideChunk(Chunk);
			}
		}
	}
}

void FVoxelDefaultRenderer::ProcessMeshUpdates(double MaxTime)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelTaskCallback Callback;
	while ( // First check the time, else dequeued elements aren't processed!
		FPlatformTime::Seconds() < MaxTime &&
		TasksCallbacksQueue.Dequeue(Callback))
	{
		FChunk* Chunk = ChunksMap.Find(Callback.ChunkId);
		if (!Chunk) continue;

		auto& Tasks = Chunk->Tasks;
		auto& Task = Callback.bIsTransitionTask ? Tasks.TransitionsTask : Tasks.MainTask;
		if (!Task.IsValid() || Task->TaskId != Callback.TaskId) continue; // If task was canceled
		if (!ensure(Task->IsDone())) continue; // Must be done if we're in the callback

		// Move built data
		auto& BuiltData = Chunk->BuiltData;
		const auto PreviousBuiltData = BuiltData;
		if (Callback.bIsTransitionTask)
		{
			ensure(Task->TransitionsMask == Chunk->Settings.TransitionsMask); // Should have been canceled
			BuiltData.TransitionsMask = Task->TransitionsMask;
			BuiltData.TransitionsChunk = Task->Chunk;
			BuiltData.TransitionsChunkCreationTime = Task->CreationTime;
		}
		else
		{
			BuiltData.MainChunk = Task->Chunk;
			BuiltData.MainChunkCreationTime = Task->CreationTime;
		}

		// Finally, delete the task
		Task.Reset();

		// Do nothing while the main chunk isn't valid - we don't want to have unneeded updates for transitions then main
		if (BuiltData.MainChunk.IsValid())
		{
			auto& MeshId = Chunk->MeshId;
			const auto Update = [&]()
			{
				if (!MeshId.IsValid())
				{
					MeshId = MeshHandler->AddChunk(Chunk->LOD, Chunk->Bounds.Min);
				}
				MeshHandler->UpdateChunk(MeshId, Chunk->Settings, *BuiltData.MainChunk, BuiltData.TransitionsChunk.Get(), BuiltData.TransitionsMask);

				if (Settings.bStaticWorld)
				{
					// Free up memory ASAP
					BuiltData.MainChunk.Reset();
					BuiltData.TransitionsChunk.Reset();
				}
			};

			const bool bTransitionsChunkIsBuilt =
				BuiltData.TransitionsChunk.IsValid() ||
				Chunk->Settings.TransitionsMask == 0 ||
				Settings.RenderType == EVoxelRenderType::SurfaceNets;

			if (BuiltData.MainChunk->IsEmpty() && (!BuiltData.TransitionsChunk.IsValid() || BuiltData.TransitionsChunk->IsEmpty()))
			{
				// Both empty, remove mesh if existing
				if (MeshId.IsValid())
				{
					MeshHandler->RemoveChunk(MeshId);
					MeshId = {};
				}
			}
			else
			{
				Update();
				
				ensure(MeshId.IsValid());

				// Dither in if first update
				// If first load and LOD 0, don't dither as it doesn't look nice to have the world dithering under the player
				if (Settings.bDitherChunks &&
					!PreviousBuiltData.MainChunk.IsValid() && 
					!(UpdateIndex == 1 && Chunk->LOD == 0))
				{
					// Can be a first update if:
					// - we are a showed new chunks that's dithering in
					// - we are a hidden chunk that's updated for the first time. If so don't dither in
					ensure(Chunk->GetState() == EChunkState::Hidden || Chunk->GetState() == EChunkState::DitheringIn);
					if (Chunk->GetState() == EChunkState::DitheringIn)
					{
						DitherInChunk(*Chunk, Chunk->PreviousChunks);
					}
				}
			}

			// Dither out/remove previous chunks only once transitions are built too
			// Note: bTransitionsChunkIsBuilt is always true for surface nets
			if (bTransitionsChunkIsBuilt)
			{
				ClearPreviousChunks(*Chunk);
			}
		}
		else
		{
			ensure(!Chunk->MeshId.IsValid());
		}

		// Start new tasks as needed
		CheckPendingUpdates(*Chunk);
	}
}

void FVoxelDefaultRenderer::FlushQueuedTasks()
{
	VOXEL_FUNCTION_COUNTER();

	const auto Flush = [&](bool bVisible, bool bHasCollisions)
	{
		auto& Tasks = QueuedTasks[bVisible][bHasCollisions];
		if (Tasks.Num() > 0)
		{
			TaskCount.Add(Tasks.Num());
			const auto TaskType =
				bVisible
				? bHasCollisions
				? EVoxelTaskType::VisibleCollisionsChunksMeshing
				: EVoxelTaskType::VisibleChunksMeshing
				: bHasCollisions
				? EVoxelTaskType::CollisionsChunksMeshing
				: EVoxelTaskType::ChunksMeshing;
			Settings.Pool->QueueTasks(TaskType, Tasks);
			Tasks.Reset();
		}
	};
	Flush(false, false);
	Flush(false, true);
	Flush(true, false);
	Flush(true, true);
}

void FVoxelDefaultRenderer::DestroyChunk(FChunk& Chunk)
{
	VOXEL_FUNCTION_COUNTER();

	ensure(!Chunk.MeshId.IsValid());
	ensure(Chunk.PreviousChunks.Num() == 0);
	ensureVoxelSlowNoSideEffects(!ChunksToRemove.FindByPredicate([&](const FChunkToRemove& ChunkToRemove) { return ChunkToRemove.Id == Chunk.Id; }));
	ensureVoxelSlowNoSideEffects(!ChunksToShow.FindByPredicate([&](const FChunkToShow& ChunkToShow) { return ChunkToShow.Id == Chunk.Id; }));

	if (!ensure(!Chunk.Tasks.MainTask.IsValid()) || 
		!ensure(!Chunk.Tasks.TransitionsTask.IsValid()))
	{
		CancelTasks(Chunk);
	}
	
	for (auto& PendingUpdate : Chunk.PendingUpdates)
	{
		// We must always fire all delegates
		PendingUpdate.OnUpdateFinished.Broadcast(FVoxelIntBox());
	}
	ensure(ChunksMap.Remove(Chunk.Id) == 1);
}

void FVoxelDefaultRenderer::UpdateAllocatedSize()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderer, AllocatedSize);

	AllocatedSize = 0;
	AllocatedSize += ChunksMap.GetAllocatedSize();
	AllocatedSize += ChunksToRemove.GetAllocatedSize();
	AllocatedSize += ChunksToShow.GetAllocatedSize();
	
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelRenderer, AllocatedSize);
}

void FVoxelDefaultRenderer::CancelTask(TUniquePtr<FVoxelMesherAsyncWork, TVoxelAsyncWorkDelete<FVoxelMesherAsyncWork>>& Task)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(Task.IsValid());
	
	const bool bIsDone = Task->CancelAndAutodelete();
	Task.Release();
	if (!bIsDone)
	{
		// If IsDone, QueueChunkCallback_AnyThread was called
		ensure(TaskCount.Decrement() >= 0);
	}
}

void FVoxelDefaultRenderer::QueueChunkCallback_AnyThread(uint64 TaskId, uint64 ChunkId, bool bIsTransitionTask)
{
	ensure(TaskCount.Decrement() >= 0);
	TasksCallbacksQueue.Enqueue({TaskId, ChunkId, bIsTransitionTask});
}