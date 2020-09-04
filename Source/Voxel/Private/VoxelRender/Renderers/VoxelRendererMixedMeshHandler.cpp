// Copyright 2020 Phyronnaz

#include "VoxelRendererMixedMeshHandler.h"
#include "VoxelRendererBasicMeshHandler.h"
#include "VoxelRendererClusteredMeshHandler.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"

FVoxelRendererMixedMeshHandler::FVoxelRendererMixedMeshHandler(IVoxelRenderer& Renderer)
	: IVoxelRendererMeshHandler(Renderer)
	, BasicMeshHandler(MakeVoxelShared<FVoxelRendererBasicMeshHandler>(Renderer))
	, ClusteredMeshHandler(MakeVoxelShared<FVoxelRendererClusteredMeshHandler>(Renderer))
{
	BasicMeshHandler->Init();
	ClusteredMeshHandler->Init();
}

FVoxelRendererMixedMeshHandler::~FVoxelRendererMixedMeshHandler()
{
	ensure(ChunkInfos.Num() == 0);
}

IVoxelRendererMeshHandler::FChunkId FVoxelRendererMixedMeshHandler::AddChunkImpl(int32 LOD, const FIntVector& Position)
{
	return ChunkInfos.Add(FChunkInfo(LOD, Position));
}

void FVoxelRendererMixedMeshHandler::ApplyAction(const FAction& Action)
{
	VOXEL_FUNCTION_COUNTER();

	switch (Action.Action)
	{
	case EAction::UpdateChunk:
	{
		const auto& UpdateChunk = Action.UpdateChunk().InitialCall;
		const auto ChunkSettings = UpdateChunk.ChunkSettings;
		const bool bNeedBasicChunk = ChunkSettings.bEnableCollisions || ChunkSettings.bEnableNavmesh;
		const bool bNeedClusteredChunk = ChunkSettings.bVisible;
		auto& ChunkInfo = ChunkInfos[Action.ChunkId];

		if (bNeedBasicChunk)
		{
			if (!ChunkInfo.BasicChunkId.IsValid())
			{
				ChunkInfo.BasicChunkId = BasicMeshHandler->AddChunk(ChunkInfo.LOD, ChunkInfo.Position);
			}
			auto BasicChunkSettings = ChunkSettings;
			BasicChunkSettings.bVisible = false;
			BasicMeshHandler->UpdateChunk(
				ChunkInfo.BasicChunkId,
				BasicChunkSettings,
				*UpdateChunk.MainChunk,
				nullptr,
				0);
		}
		else
		{
			if (ChunkInfo.BasicChunkId.IsValid())
			{
				BasicMeshHandler->RemoveChunk(ChunkInfo.BasicChunkId);
				ChunkInfo.BasicChunkId = {};
			}
		}

		if (bNeedClusteredChunk)
		{
			if (!ChunkInfo.ClusteredChunkId.IsValid())
			{
				ChunkInfo.ClusteredChunkId = ClusteredMeshHandler->AddChunk(ChunkInfo.LOD, ChunkInfo.Position);
			}
			auto ClusteredChunkSettings = ChunkSettings;
			ClusteredChunkSettings.bEnableCollisions = false;
			ClusteredChunkSettings.bEnableNavmesh = false;
			ClusteredMeshHandler->UpdateChunk(
				ChunkInfo.ClusteredChunkId,
				ClusteredChunkSettings,
				*UpdateChunk.MainChunk,
				UpdateChunk.TransitionChunk,
				ChunkSettings.TransitionsMask);
		}
		else
		{
			if (ChunkInfo.ClusteredChunkId.IsValid())
			{
				ClusteredMeshHandler->RemoveChunk(ChunkInfo.ClusteredChunkId);
				ChunkInfo.ClusteredChunkId = {};
			}
		}

		break;
	}
	case EAction::RemoveChunk:
	{
		auto& ChunkInfo = ChunkInfos[Action.ChunkId];
		if (ChunkInfo.BasicChunkId.IsValid())
		{
			BasicMeshHandler->RemoveChunk(ChunkInfo.BasicChunkId);
		}
		if (ChunkInfo.ClusteredChunkId.IsValid())
		{
			ClusteredMeshHandler->RemoveChunk(ChunkInfo.ClusteredChunkId);
		}
		ChunkInfos.RemoveAt(Action.ChunkId);
		break;
	}
	case EAction::SetTransitionsMaskForSurfaceNets:
	{
		break;
	}
	case EAction::DitherChunk:
	case EAction::ResetDithering:
	// These 2 should be done by an UpdateChunk
	case EAction::HideChunk:
	case EAction::ShowChunk:
	default: ensure(false);
	}
}

void FVoxelRendererMixedMeshHandler::ClearChunkMaterials()
{
	BasicMeshHandler->ClearChunkMaterials();
	ClusteredMeshHandler->ClearChunkMaterials();
}

void FVoxelRendererMixedMeshHandler::Tick(double MaxTime)
{
	VOXEL_FUNCTION_COUNTER();

	IVoxelRendererMeshHandler::Tick(MaxTime);

	BasicMeshHandler->Tick(MaxTime);
	ClusteredMeshHandler->Tick(MaxTime);
}

void FVoxelRendererMixedMeshHandler::RecomputeMeshPositions()
{
	VOXEL_FUNCTION_COUNTER();

	IVoxelRendererMeshHandler::RecomputeMeshPositions();

	BasicMeshHandler->RecomputeMeshPositions();
	ClusteredMeshHandler->RecomputeMeshPositions();
}

void FVoxelRendererMixedMeshHandler::ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda)
{
	VOXEL_FUNCTION_COUNTER();

	IVoxelRendererMeshHandler::ApplyToAllMeshes(Lambda);

	BasicMeshHandler->ApplyToAllMeshes(Lambda);
	ClusteredMeshHandler->ApplyToAllMeshes(Lambda);
}

void FVoxelRendererMixedMeshHandler::StartDestroying()
{
	VOXEL_FUNCTION_COUNTER();

	IVoxelRendererMeshHandler::StartDestroying();

	BasicMeshHandler->StartDestroying();
	ClusteredMeshHandler->StartDestroying();
}