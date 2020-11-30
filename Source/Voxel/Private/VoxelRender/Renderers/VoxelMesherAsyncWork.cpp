// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelMesherAsyncWork.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelRender/Meshers/VoxelMarchingCubeMesher.h"
#include "VoxelRender/Meshers/VoxelCubicMesher.h"
#include "VoxelRender/Meshers/VoxelGreedyCubicMesher.h"
#include "VoxelRender/Meshers/VoxelSurfaceNetMesher.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelData/VoxelDataSubsystem.h"

#include "Async/Async.h"
#include "Misc/MessageDialog.h"

FVoxelMesherAsyncWork::FVoxelMesherAsyncWork(
	FVoxelDefaultRenderer& Renderer,
	const uint64 ChunkId,
	const int32 LOD,
	const FVoxelIntBox& Bounds,
	const bool bIsTransitionTask,
	const uint8 TransitionsMask,
	EVoxelTaskType TaskType)
	: FVoxelAsyncWork(STATIC_FNAME("FVoxelMesherAsyncWork"), TaskType, EPriority::InvokersDistance)
	, ChunkId(ChunkId)
	, LOD(LOD)
	, ChunkPosition(Bounds.Min)
	, bIsTransitionTask(bIsTransitionTask)
	, TransitionsMask(TransitionsMask)
	, Renderer(Renderer.AsShared())
{
	check(IsInGameThread());
	ensure(!bIsTransitionTask || TransitionsMask != 0);

	PriorityHandler = FVoxelPriorityHandler(Bounds, Renderer);
}

FVoxelMesherAsyncWork::~FVoxelMesherAsyncWork()
{
}

static void ShowGeneratorError(TVoxelWeakPtr<const FVoxelData> Data)
{
	static TSet<TVoxelWeakPtr<const FVoxelData>> IgnoredDatas;
	if (!IgnoredDatas.Contains(Data))
	{
		const auto Result = FMessageDialog::Open(
			EAppMsgType::YesNo,
			VOXEL_LOCTEXT(
				"Error: The generator is returning different values for the same position/LOD.\n"
				"Please check your code.\n"
				"If you're using a voxel graph, this is an internal error, please report it to the developer.\n"
				"Hide future errors?"));

		if (Result == EAppReturnType::Yes)
		{
			IgnoredDatas.Add(Data);
		}
	}
}

void FVoxelMesherAsyncWork::DoWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const auto PinnedRenderer = Renderer.Pin();
	if (IsCanceled()) return;
	if (!ensure(PinnedRenderer.IsValid())) return; // Either we're canceled, or the renderer is valid

	const auto DataSubsystem = PinnedRenderer->GetSubsystem<FVoxelDataSubsystem>();
	if (IsCanceled()) return;
	if (!DataSubsystem.IsValid()) return; // Happens when the renderer is still canceling tasks

	const auto Mesher = GetMesher(
		ChunkPosition,
		*PinnedRenderer,
		DataSubsystem->GetData(),
		LOD,
		bIsTransitionTask,
		TransitionsMask);

	CreationTime = FPlatformTime::Seconds();

	if (PinnedRenderer->Settings.bRenderWorld)
	{
		const auto MesherChunk = Mesher->CreateFullChunk();
		if (MesherChunk.IsValid())
		{
			Chunk = MesherChunk.ToSharedRef();
		}
		else
		{
			AsyncTask(ENamedThreads::GameThread, [Data = MakeVoxelWeakPtr(DataSubsystem->GetDataPtr())]() { ShowGeneratorError(Data); });
			Chunk = Mesher->CreateEmptyChunk();
		}
	}
	else
	{
		// If we're not rendering the world, we only need the geometry for collisions/navmesh

		TArray<uint32> Indices;
		TArray<FVector> Vertices;
		Mesher->CreateGeometry(Indices, Vertices);
		
		Chunk = MakeVoxelShared<FVoxelChunkMesh>();
		Chunk->SetIsSingle(true);
		FVoxelChunkMeshBuffers& Buffers = Chunk->CreateSingleBuffers();

		Buffers.Indices = MoveTemp(Indices);
		Buffers.Positions = MoveTemp(Vertices);
	}
}

void FVoxelMesherAsyncWork::PostDoWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	auto RendererPtr = Renderer.Pin();
	if (ensure(RendererPtr.IsValid()))
	{
		RendererPtr->QueueChunkCallback_AnyThread(TaskId, ChunkId, bIsTransitionTask);
	}
}

TUniquePtr<FVoxelMesherBase> FVoxelMesherAsyncWork::GetMesher(
	const FIntVector& ChunkPosition,
	const IVoxelRenderer& Renderer,
	const FVoxelData& Data,
	int32 LOD,
	bool bIsTransitionTask,
	uint8 TransitionsMask)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	switch (Renderer.Settings.RenderType)
	{
	default:
	case EVoxelRenderType::MarchingCubes:
	{
		if (bIsTransitionTask)
		{
			return MakeUnique<FVoxelMarchingCubeTransitionsMesher>(LOD, ChunkPosition, Renderer, Data, TransitionsMask);
		}
		else
		{
			return MakeUnique<FVoxelMarchingCubeMesher>(LOD, ChunkPosition, Renderer, Data);
		}
	}
	case EVoxelRenderType::Cubic:
	{
		if (bIsTransitionTask)
		{
			return MakeUnique<FVoxelCubicTransitionsMesher>(LOD, ChunkPosition, Renderer, Data, TransitionsMask);
		}
		else
		{
			if (Renderer.Settings.bGreedyCubicMesher)
			{
				return MakeUnique<FVoxelGreedyCubicMesher>(LOD, ChunkPosition, Renderer, Data);
			}
			else
			{
				return MakeUnique<FVoxelCubicMesher>(LOD, ChunkPosition, Renderer, Data);
			}
		}
	}
	case EVoxelRenderType::SurfaceNets:
	{
		if (bIsTransitionTask)
		{
			check(false);
			return nullptr;
		}
		else
		{
			return MakeUnique<FVoxelSurfaceNetMesher>(LOD, ChunkPosition, Renderer, Data);
		}
	}
	}
}

void FVoxelMesherAsyncWork::CreateGeometry_AnyThread(
	const FIntVector& ChunkPosition,
	const IVoxelRenderer& Renderer,
	const FVoxelData& Data,
	int32 LOD,
	TArray<uint32>& OutIndices,
	TArray<FVector>& OutVertices)
{
	const auto Mesher = GetMesher(ChunkPosition, Renderer, Data, LOD, false, 0);
	Mesher->CreateGeometry(OutIndices, OutVertices);
}