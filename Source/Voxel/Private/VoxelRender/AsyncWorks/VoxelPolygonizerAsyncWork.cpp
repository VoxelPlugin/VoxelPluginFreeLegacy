// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelRender/Renderers/VoxelRenderChunk.h"
#include "VoxelRender/Polygonizers/VoxelPolygonizer.h"
#include "VoxelData/VoxelData.h"

#include "Async/Async.h"
#include "Misc/ScopeLock.h"
#include "Misc/MessageDialog.h"

FVoxelPolygonizerAsyncWorkBase::FVoxelPolygonizerAsyncWorkBase(FVoxelRenderChunk* Chunk, bool bIsTransitionsTask)
	: FVoxelAsyncWork(
		*FString::Printf(TEXT("PolygonizerWork LOD=%d Transitions=%d"), Chunk->LOD, bIsTransitionsTask ? 1 : 0),
		Chunk->GetPriority(),
		FVoxelAsyncWorkCallback::CreateStatic(FVoxelPolygonizerAsyncWorkBase::Callback, TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe>(Chunk->AsShared()), bIsTransitionsTask, Chunk->GetTaskId()))
	, LOD           (Chunk->LOD)
	, ChunkPosition (Chunk->Position)
	, Data          (Chunk->Renderer->Settings.Data.ToSharedRef())
	, DebugManager  (Chunk->Renderer->Settings.DebugManager)
	, VoxelSize     (Chunk->Renderer->Settings.VoxelSize)
	, UVConfig      (Chunk->Renderer->Settings.UVConfig)
	, NormalConfig  (Chunk->Renderer->Settings.NormalConfig)
	, MaterialConfig(Chunk->Renderer->Settings.MaterialConfig)
	, MeshParameters(Chunk->Settings.bEnableTessellation, Chunk->Renderer->Settings.bOptimizeIndices)
{
	check(IsInGameThread());
	Stats.StartStat("Waiting for thread queue", false);
}

FVoxelPolygonizerAsyncWorkBase::~FVoxelPolygonizerAsyncWorkBase()
{
	if (IsDone() && !IsCanceled())
	{
		FVoxelStats::AddElement(Stats);
	}
}

#define LOCTEXT_NAMESPACE "Voxel"

void ShowWorldGeneratorError(FVoxelData* Data)
{
	static TSet<FVoxelData*> IgnoredDatas;
	if (!IgnoredDatas.Contains(Data))
	{
		auto Result = FMessageDialog::Open(
			EAppMsgType::YesNo,
			LOCTEXT("WorldGeneratorError",
				"Error: The world generator is returning different values for the same position/LOD.\n"
				"Please check your code.\n"
				"If you're using a voxel graph, this is an internal error, please report it to the developer.\n"
				"Hide future errors?"));

		if (Result == EAppReturnType::Yes)
		{
			IgnoredDatas.Add(Data);
		}
	}
}

#undef LOCTEXT_NAMESPACE

void FVoxelPolygonizerAsyncWorkBase::DoWork()
{
	Stats.SetLOD(LOD);
	Stats.SetType(GetTaskType());

	Stats.StartStat("Polygonizer Creation");
	TSharedPtr<FVoxelPolygonizerBase> Polygonizer = GetPolygonizer();

	if (!Polygonizer->Create())
	{
		auto* DataPtr = &Data.Get();
		AsyncTask(ENamedThreads::GameThread, [=]() { ShowWorldGeneratorError(DataPtr); });
	}

	Stats.SetIsCanceled(IsCanceled());

	if (!IsCanceled())
	{
		PostMeshCreation();
	}

	Stats.StartStat("Waiting for game thread", false);
}

void FVoxelPolygonizerAsyncWorkBase::Callback(TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> Chunk, bool bIsTransitionsTask, uint64 TaskId)
{
	AsyncTask(ENamedThreads::GameThread, [=]()
	{
		auto ChunkPtr = Chunk.Pin();
		if (ChunkPtr.IsValid())
		{
			if (bIsTransitionsTask)
			{
				ChunkPtr->TransitionsCallback();
			}
			else
			{
				ChunkPtr->MeshCallback(TaskId);
			}
		}
	});
}

FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
	: FVoxelPolygonizerAsyncWorkBase(Chunk, false)
{
}


FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
	: FVoxelPolygonizerAsyncWorkBase(Chunk, true)
	, TransitionsMask(Chunk->GetWantedTransitionsMask())
{

}
