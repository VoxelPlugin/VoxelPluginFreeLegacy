// Copyright 2020 Phyronnaz

#include "VoxelRendererMeshHandler.h"
#include "VoxelMathUtilities.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelRenderUtilities.h"

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Proc Mesh Pool"), STAT_VoxelProcMeshPool, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Proc Mesh Used"), STAT_VoxelProcMeshUsed, STATGROUP_VoxelMemory);

TAutoConsoleVariable<int32> CVarLogActionQueue(
	TEXT("voxel.renderer.LogMeshActionQueue"),
	0,
	TEXT("If true, will log every queued action when processed"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarLogMeshPositionsPrecisionsErrors(
	TEXT("voxel.renderer.LogMeshPositionsPrecisionsErrors"),
	0,
	TEXT("If true, will log mesh positions precisions errors"),
	ECVF_Default);

IVoxelRendererMeshHandler::IVoxelRendererMeshHandler(IVoxelRenderer& Renderer)
	: Renderer(Renderer)
{
}

IVoxelRendererMeshHandler::~IVoxelRendererMeshHandler()
{
	VOXEL_FUNCTION_COUNTER();
	ensure(bIsDestroying);
	
	// Avoid crashes
	if (GExitPurge) return;
	
#if CHECK_CHUNK_IDS
	ensure(ValidIndices.Num() == 0);
#endif
	
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshUsed, ActiveMeshes.Num());
	DEC_DWORD_STAT_BY(STAT_VoxelProcMeshPool, MeshPool.Num());

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
		// Is raised when recompiling a voxel world BP, so ensureVoxelSlow and not ensure
		// ensureVoxelSlow(NumInvalid == 0 || GExitPurge);
		ensureVoxelSlow(ActiveMeshes.Num() == 0);
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

void IVoxelRendererMeshHandler::RecomputeMeshPositions()
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!bIsDestroying);

	for (auto& It : ActiveMeshes)
	{
		if (ensure(It.Key.IsValid()))
		{
			SetMeshPosition(*It.Key, It.Value);
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

	auto& Settings = Renderer.Settings;
	
	auto* const RootComponent = Settings.RootComponent.Get();
	if (!ensure(RootComponent))
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
			NewMesh = NewObject<UVoxelProceduralMeshComponent>(RootComponent, Settings.ProcMeshClass, NAME_None, RF_Transient);
			NewMesh->bCastFarShadow = Settings.bCastFarShadow;
			NewMesh->SetupAttachment(RootComponent, NAME_None);
			auto* Root = Cast<UPrimitiveComponent>(RootComponent);
			if (ensure(Root))
			{
				NewMesh->BodyInstance.CopyRuntimeBodyInstancePropertiesFrom(&Root->BodyInstance);
			}
			NewMesh->RegisterComponent();
			NewMesh->SetRelativeScale3D(FVector::OneVector * Settings.VoxelSize);
		}
	}
	check(NewMesh);

	INC_DWORD_STAT(STAT_VoxelProcMeshUsed);

	ensure(!ActiveMeshes.Contains(NewMesh));
	ActiveMeshes.Add(NewMesh, Position);
	SetMeshPosition(*NewMesh, Position);
	
	const FIntBox Bounds = FVoxelUtilities::GetBoundsFromPositionAndDepth<RENDER_CHUNK_SIZE>(Position, LOD);
	const FVoxelPriorityHandler PriorityHandler(Bounds, Renderer.GetInvokersPositions());

	// Set mesh variables
	NewMesh->Init(
		LOD,
		ChunkId.GetDebugValue(),
		PriorityHandler,
		AsShared(),
		Settings);

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

		// Set world location to 0 to avoid precision issues, as SetRelativeLocation calls MoveComponent :(
#if ENGINE_MINOR_VERSION < 24
		check(!Mesh.bAbsoluteLocation);
		Mesh.bAbsoluteLocation = true;
		Mesh.SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator::ZeroRotator);
		Mesh.bAbsoluteLocation = false;
#else
		Mesh.SetUsingAbsoluteLocation(true);
		Mesh.SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator::ZeroRotator);
		Mesh.SetUsingAbsoluteLocation(false);
#endif
	}

	MeshPool.Add(&Mesh);

	DEC_DWORD_STAT(STAT_VoxelProcMeshUsed);
	INC_DWORD_STAT(STAT_VoxelProcMeshPool);
}

TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& IVoxelRendererMeshHandler::CleanUp(TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& Meshes) const
{
	const int32 NumInvalid = Meshes.RemoveAll([](auto& Mesh) { return !Mesh.IsValid(); });
	// Meshes are invalid when closing the editor
	// ensureVoxelSlow(NumInvalid == 0 || GExitPurge);
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

void IVoxelRendererMeshHandler::SetMeshPosition(UVoxelProceduralMeshComponent& Mesh, const FIntVector& Position) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(!bIsDestroying);

	// TODO errors might add up when we rebase?
	Mesh.SetRelativeLocationAndRotation(
		Renderer.Settings.GetChunkRelativePosition(Position),
		FRotator::ZeroRotator,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	// If we don't do that the component does not update if Position = 0 0 0 :(
	// Probably UE bug?
	if (Position == FIntVector(0, 0, 0))
	{
		Mesh.UpdateComponentToWorld(EUpdateTransformFlags::None, ETeleportType::TeleportPhysics);
	}

	if (CVarLogMeshPositionsPrecisionsErrors.GetValueOnGameThread() != 0)
	{
		const auto A = Mesh.GetRelativeTransform().GetTranslation();
		const auto B = Renderer.Settings.GetChunkRelativePosition(Position);
		const float Error = FVector::Distance(A, B);
		if (Error > 0)
		{
			UE_LOG(LogVoxel, Log, TEXT("Distance between theorical and actual mesh position: %6.6f voxels"), Error);
			ensure(Error < 1);
		}
	}
}