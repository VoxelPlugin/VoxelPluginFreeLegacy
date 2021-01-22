// Copyright 2021 Phyronnaz

#include "VoxelRendererMeshHandler.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelWorldRootComponent.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Proc Mesh Pool"), STAT_VoxelProcMeshPool, STATGROUP_VoxelCounters);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Proc Mesh Frozen Pool"), STAT_VoxelProcMeshFrozenPool, STATGROUP_VoxelCounters);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Proc Mesh Used"), STAT_VoxelProcMeshUsed, STATGROUP_VoxelCounters);

TAutoConsoleVariable<int32> CVarLogActionQueue(
	TEXT("voxel.renderer.LogMeshActionQueue"),
	0,
	TEXT("If true, will log every queued action when processed"),
	ECVF_Default);

IVoxelRendererMeshHandler::IVoxelRendererMeshHandler(IVoxelRenderer& Renderer)
	: Renderer(Renderer)
{
}

IVoxelRendererMeshHandler::~IVoxelRendererMeshHandler()
{
	VOXEL_FUNCTION_COUNTER();
	check(bIsInit);
	check(IsInGameThread());
	ensure(bIsDestroying);
	
	// Avoid crashes
	if (GExitPurge) return;
	
#if CHECK_CHUNK_IDS
	ensure(ValidIndices.Num() == 0);
#endif
	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshUsed, ActiveMeshes.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshPool, MeshPool.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshFrozenPool, FrozenMeshPool.Num());

#if VOXEL_DEBUG
	{
		int32 NumInvalid = 0;

		for (auto It = ActiveMeshes.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
				NumInvalid++;
			}
		}
		
		// Meshes are invalid when closing the editor
		// Is raised when recompiling a voxel world BP, so ensureVoxelSlowNoSideEffects and not ensure
		// ensureVoxelSlowNoSideEffects(NumInvalid == 0 || GExitPurge);
		ensureVoxelSlowNoSideEffects(ActiveMeshes.Num() == 0);
	}
#endif

	VOXEL_SCOPE_COUNTER("Destroy proc mesh components");
	// Destroy all mesh components we are owning
	for (auto& Mesh : MeshPool)
	{
		if (Mesh.IsValid())
		{
			Mesh->DestroyComponent();
		}
	}
	for (auto& Mesh : FrozenMeshPool)
	{
		if (Mesh.IsValid())
		{
			Mesh->DestroyComponent();
		}
	}
}

IVoxelRendererMeshHandler::FChunkId IVoxelRendererMeshHandler::AddChunk(int32 LOD, const FIntVector& Position)
{
	VOXEL_FUNCTION_COUNTER();
	
	const FChunkId Id = AddChunkImpl(LOD, Position);
#if CHECK_CHUNK_IDS
	ValidIndices.Add(Id);
#endif
	return Id;
}

#if CHECK_CHUNK_IDS
#define CHECK_CHUNK_ID_IMPL(R) if (!ensure(ChunkId.IsValid()) || !ensure(ValidIndices.Contains(ChunkId))) return R;
#define CHECK_CHUNK_ID() CHECK_CHUNK_ID_IMPL(;)
#define CHECK_CHUNK_ID_RETURN() CHECK_CHUNK_ID_IMPL({})
#else
#define CHECK_CHUNK_ID()
#define CHECK_CHUNK_ID_RETURN()
#endif

void IVoxelRendererMeshHandler::UpdateChunk(
	FChunkId ChunkId,
	const FVoxelChunkSettings& ChunkSettings,
	const FVoxelChunkMesh& MainChunk,
	const FVoxelChunkMesh* TransitionChunk,
	uint8 TransitionsMask)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();

	ensure(ChunkSettings.bVisible || ChunkSettings.bEnableCollisions || ChunkSettings.bEnableNavmesh);
	
	FAction Action;
	Action.Action = EAction::UpdateChunk;
	Action.ChunkId = ChunkId;
	Action.UpdateChunk().InitialCall.ChunkSettings = ChunkSettings;
	Action.UpdateChunk().InitialCall.ChunkSettings.TransitionsMask = TransitionsMask; // Make subclass believe this is the transitions mask
	Action.UpdateChunk().InitialCall.MainChunk = &MainChunk;
	Action.UpdateChunk().InitialCall.TransitionChunk = TransitionChunk;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::RemoveChunk(FChunkId ChunkId)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();
	
#if CHECK_CHUNK_IDS
	ValidIndices.Remove(ChunkId);
#endif

	FAction Action;
	Action.Action = EAction::RemoveChunk;
	Action.ChunkId = ChunkId;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::DitherChunk(FChunkId ChunkId, EDitheringType DitheringType)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();
	
	FAction Action;
	Action.Action = EAction::DitherChunk;
	Action.ChunkId = ChunkId;
	Action.DitherChunk().DitheringType = DitheringType;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::ResetDithering(FChunkId ChunkId)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();
	
	FAction Action;
	Action.Action = EAction::ResetDithering;
	Action.ChunkId = ChunkId;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::SetTransitionsMaskForSurfaceNets(FChunkId ChunkId, uint8 TransitionsMask)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();
	
	FAction Action;
	Action.Action = EAction::SetTransitionsMaskForSurfaceNets;
	Action.ChunkId = ChunkId;
	Action.SetTransitionsMaskForSurfaceNets().TransitionsMask = TransitionsMask;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::HideChunk(FChunkId ChunkId)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();

	// Clustered renderer does not support those
	ensure(Renderer.Settings.bDitherChunks);
	
	FAction Action;
	Action.Action = EAction::HideChunk;
	Action.ChunkId = ChunkId;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::ShowChunk(FChunkId ChunkId)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_CHUNK_ID();
	
	// Clustered renderer does not support those
	ensure(Renderer.Settings.bDitherChunks);
	
	FAction Action;
	Action.Action = EAction::ShowChunk;
	Action.ChunkId = ChunkId;
	ApplyAction(Action);
}

void IVoxelRendererMeshHandler::Tick(double MaxTime)
{
	TickHandler();
}

void IVoxelRendererMeshHandler::RecomputeComponentPositions()
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!bIsDestroying);

	for (auto& It : ActiveMeshes)
	{
		UVoxelProceduralMeshComponent* Component = It.Key.Get();
		if (ensure(Component))
		{
			Renderer.Settings.SetComponentPosition(*Component, It.Value, true);
		}
	}
}

void IVoxelRendererMeshHandler::ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!bIsDestroying);

	for (auto& It : ActiveMeshes)
	{
		if (ensureVoxelSlow(It.Key.IsValid()))
		{
			Lambda(*It.Key);
		}
	}
}

void IVoxelRendererMeshHandler::StartDestroying()
{
	ensure(!bIsDestroying);
	bIsDestroying = true;
}

UVoxelProceduralMeshComponent* IVoxelRendererMeshHandler::GetNewMesh(FChunkId ChunkId, const FIntVector& Position, uint8 LOD)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!bIsDestroying);

	const FVoxelRuntimeSettings& Settings = Renderer.Settings;

	AActor* ComponentsOwner = Settings.ComponentsOwner.Get();
	UVoxelWorldRootComponent* VoxelRootComponent = Settings.VoxelRootComponent.Get();
	
	if (!ensureVoxelSlow(ComponentsOwner) ||
		!ensureVoxelSlow(VoxelRootComponent))
	{
		return nullptr;
	}

	UVoxelProceduralMeshComponent* NewMesh = nullptr;
	{
		while (MeshPool.Num() > 0 && !NewMesh)
		{
			NewMesh = MeshPool.Pop(false).Get();
			ensure(NewMesh != nullptr);
			DEC_DWORD_STAT(STAT_VoxelProcMeshPool);
		}

		if (!NewMesh)
		{
			NewMesh = NewObject<UVoxelProceduralMeshComponent>(ComponentsOwner, Settings.ProcMeshClass, NAME_None, RF_Transient);
			Settings.SetupComponent(*NewMesh);
			
			NewMesh->bCastFarShadow = Settings.bCastFarShadow;

			NewMesh->BodyInstance.CopyRuntimeBodyInstancePropertiesFrom(&VoxelRootComponent->BodyInstance);
			NewMesh->BodyInstance.SetObjectType(VoxelRootComponent->BodyInstance.GetObjectType());
			NewMesh->SetGenerateOverlapEvents(VoxelRootComponent->GetGenerateOverlapEvents());
			NewMesh->RuntimeVirtualTextures = VoxelRootComponent->RuntimeVirtualTextures;
			NewMesh->VirtualTextureLodBias = VoxelRootComponent->VirtualTextureLodBias;
			NewMesh->VirtualTextureCullMips = VoxelRootComponent->VirtualTextureCullMips;
			NewMesh->VirtualTextureMinCoverage = VoxelRootComponent->VirtualTextureMinCoverage;
			NewMesh->VirtualTextureRenderPassType = VoxelRootComponent->VirtualTextureRenderPassType;

			NewMesh->RegisterComponent();
		}
	}
	check(NewMesh);

	INC_DWORD_STAT(STAT_VoxelProcMeshUsed);

	ensure(!ActiveMeshes.Contains(NewMesh));
	ActiveMeshes.Add(NewMesh, Position);
	Settings.SetComponentPosition(*NewMesh, Position, true);
	
	const FVoxelIntBox Bounds = FVoxelUtilities::GetBoundsFromPositionAndDepth<RENDER_CHUNK_SIZE>(Position, LOD);
	const FVoxelPriorityHandler PriorityHandler(Bounds, Renderer);

	// Set mesh variables
	NewMesh->Init(
		LOD,
		ChunkId.GetDebugValue(),
		PriorityHandler,
		AsShared(),
		Renderer);

	if (Settings.PlayType == EVoxelPlayType::Game)
	{
		// Call BP event if user want to do custom stuff
		NewMesh->InitChunk(LOD, Bounds);
	}

	return NewMesh;
}

void IVoxelRendererMeshHandler::RemoveMesh(UVoxelProceduralMeshComponent& Mesh)
{
	VOXEL_FUNCTION_COUNTER();

	// Avoid crashes
	if (GExitPurge) return;
	
	ensure(ActiveMeshes.Remove(&Mesh) == 1);

	// Skip an expensive clear when we're destroying
	if (!bIsDestroying)
	{
		Mesh.SetDistanceFieldData(nullptr);
		Mesh.ClearSections(EVoxelProcMeshSectionUpdate::UpdateNow);
		Mesh.ClearInit();
	}

	if (UVoxelProceduralMeshComponent::AreVoxelCollisionsFrozen(Renderer.Settings.VoxelWorld.Get()))
	{
		FrozenMeshPool.Add(&Mesh);
		INC_DWORD_STAT(STAT_VoxelProcMeshFrozenPool);
	}
	else
	{
		MeshPool.Add(&Mesh);
		INC_DWORD_STAT(STAT_VoxelProcMeshPool);
	}

	DEC_DWORD_STAT(STAT_VoxelProcMeshUsed);
}

TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& IVoxelRendererMeshHandler::CleanUp(TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& Meshes) const
{
	const int32 NumInvalid = Meshes.RemoveAll([](auto& Mesh) { return !Mesh.IsValid(); });
	// Meshes are invalid when closing the editor
	// ensureVoxelSlowNoSideEffects(NumInvalid == 0 || GExitPurge);
	return Meshes;
}

FString IVoxelRendererMeshHandler::FAction::ToString() const
{
	FString String = FString::Printf(TEXT("ChunkId: %u; Type: "), ChunkId.GetDebugValue());
	switch (Action)
	{
	case EAction::UpdateChunk:
	{
		String += "UpdateChunk";
		break;
	}
	case EAction::RemoveChunk:
	{
		String += "RemoveChunk";
		break;
	}
	case EAction::DitherChunk:
	{
		String += "DitherChunk; DitheringType: ";
		switch (DitherChunk().DitheringType)
		{
		case EDitheringType::SurfaceNets_LowResToHighRes: String += "SurfaceNets_LowResToHighRes"; break;
		case EDitheringType::SurfaceNets_HighResToLowRes: String += "SurfaceNets_HighResToLowRes"; break;
		case EDitheringType::Classic_DitherIn: String += "Classic_DitherIn"; break;
		case EDitheringType::Classic_DitherOut: String += "Classic_DitherOut"; break;
		default: ensure(false);
		}
		break;
	}
	case EAction::ResetDithering:
	{
		String += "ResetDithering";
		break;
	}
	case EAction::SetTransitionsMaskForSurfaceNets:
	{
		String += FString::Printf(TEXT("SetTransitionsMaskForSurfaceNets; TransitionsMask: %d"), SetTransitionsMaskForSurfaceNets().TransitionsMask);
		break;
	}
	case EAction::HideChunk:
	{
		String += "HideChunk";
		break;
	}
	case EAction::ShowChunk:
	{
		String += "ShowChunk";
		break;
	}
	default: ensure(false);
	}
	return String;
}

void IVoxelRendererMeshHandler::Init()
{
	check(!bIsInit);
	bIsInit = true;

	UVoxelProceduralMeshComponent::AddOnFreezeVoxelCollisionChanged(
		Renderer.Settings.VoxelWorld.Get(),
		FOnFreezeVoxelCollisionChanged::FDelegate::CreateThreadSafeSP(
			this,
			&IVoxelRendererMeshHandler::OnFreezeVoxelCollisionChanged));
}

void IVoxelRendererMeshHandler::OnFreezeVoxelCollisionChanged(bool bNewFreezeCollisions)
{
	if (!bNewFreezeCollisions)
	{
		// We can reuse all the frozen meshes
		// No need to do anything on them: their collisions will be unfrozen automatically by the proc mesh comp
		
		DEC_DWORD_STAT_BY(STAT_VoxelProcMeshFrozenPool, FrozenMeshPool.Num());
		INC_DWORD_STAT_BY(STAT_VoxelProcMeshPool, FrozenMeshPool.Num());

		MeshPool.Append(MoveTemp(FrozenMeshPool));
	}
}