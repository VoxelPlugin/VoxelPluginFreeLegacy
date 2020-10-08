// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelMesherAsyncWork.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelRender/Meshers/VoxelMarchingCubeMesher.h"
#include "VoxelRender/Meshers/VoxelCubicMesher.h"
#include "VoxelRender/Meshers/VoxelSurfaceNetMesher.h"
#include "VoxelRender/VoxelChunkMesh.h"

#include "Async/Async.h"
#include "Misc/MessageDialog.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"

FVoxelMesherAsyncWork::FVoxelMesherAsyncWork(
	FVoxelDefaultRenderer& Renderer,
	const uint64 ChunkId,
	const int32 LOD,
	const FVoxelIntBox& Bounds,
	const bool bIsTransitionTask,
	const uint8 TransitionsMask)
	: FVoxelAsyncWork(STATIC_FNAME("FVoxelMesherAsyncWork"), Renderer.Settings.PriorityDuration)
	, ChunkId(ChunkId)
	, LOD(LOD)
	, ChunkPosition(Bounds.Min)
	, bIsTransitionTask(bIsTransitionTask)
	, TransitionsMask(TransitionsMask)
	, Renderer(Renderer.AsShared())
	, PriorityHandler(Bounds, Renderer.GetInvokersPositionsForPriorities())
{
	check(IsInGameThread());
	ensure(!bIsTransitionTask || TransitionsMask != 0);
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

	auto PinnedRenderer = Renderer.Pin();
	if (IsCanceled()) return;
	if (!ensure(PinnedRenderer.IsValid())) return; // Either we're canceled, or the renderer is valid

	const auto Mesher = GetMesher(
		PinnedRenderer->Settings,
		LOD,
		ChunkPosition,
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
			AsyncTask(ENamedThreads::GameThread, [Data = MakeVoxelWeakPtr(PinnedRenderer->Settings.Data)]() { ShowGeneratorError(Data); });
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
	
	FVoxelUtilities::DeleteOnGameThread_AnyThread(PinnedRenderer);
}

void FVoxelMesherAsyncWork::PostDoWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	auto RendererPtr = Renderer.Pin();
	if (ensure(RendererPtr.IsValid()))
	{
		RendererPtr->QueueChunkCallback_AnyThread(TaskId, ChunkId, bIsTransitionTask);
		FVoxelUtilities::DeleteOnGameThread_AnyThread(RendererPtr);
	}
}

uint32 FVoxelMesherAsyncWork::GetPriority() const
{
	return PriorityHandler.GetPriority();
}

TUniquePtr<FVoxelMesherBase> FVoxelMesherAsyncWork::GetMesher(
	const FVoxelRendererSettings& Settings,
	int32 LOD,
	const FIntVector& ChunkPosition,
	bool bIsTransitionTask,
	uint8 TransitionsMask)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	switch (Settings.RenderType)
	{
	default:
	case EVoxelRenderType::MarchingCubes:
	{
		if (bIsTransitionTask)
		{
			return MakeUnique<FVoxelMarchingCubeTransitionsMesher>(LOD, ChunkPosition, Settings, TransitionsMask);
		}
		else
		{
			return MakeUnique<FVoxelMarchingCubeMesher>(LOD, ChunkPosition, Settings);
		}
	}
	case EVoxelRenderType::Cubic:
	{
		if (bIsTransitionTask)
		{
			return MakeUnique<FVoxelCubicTransitionsMesher>(LOD, ChunkPosition, Settings, TransitionsMask);
		}
		else
		{
			return MakeUnique<FVoxelCubicMesher>(LOD, ChunkPosition, Settings);
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
			return MakeUnique<FVoxelSurfaceNetMesher>(LOD, ChunkPosition, Settings);
		}
	}
	}
}

void FVoxelMesherAsyncWork::CreateGeometry_AnyThread(
	const FVoxelDefaultRenderer& Renderer, 
	int32 LOD, 
	const FIntVector& ChunkPosition, 
	TArray<uint32>& OutIndices, 
	TArray<FVector>& OutVertices)
{
	const auto Mesher = GetMesher(Renderer.Settings, LOD, ChunkPosition, false, 0);
	Mesher->CreateGeometry(OutIndices, OutVertices);
}