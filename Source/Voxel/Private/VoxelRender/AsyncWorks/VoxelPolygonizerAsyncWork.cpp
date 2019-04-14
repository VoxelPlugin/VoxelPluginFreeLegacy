// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRender/Renderers/VoxelDefaultRenderer.h"
#include "VoxelRender/Renderers/VoxelRenderChunk.h"
#include "VoxelRender/Polygonizers/VoxelPolygonizer.h"
#include "VoxelData/VoxelData.h"

#include "Async/Async.h"
#include "Misc/ScopeLock.h"
#include "Misc/MessageDialog.h"

FVoxelPolygonizerAsyncWorkBase::FVoxelPolygonizerAsyncWorkBase(FVoxelRenderChunk* Chunk, const FString& Name)
	: FVoxelAsyncWork(
		*FString::Printf(TEXT("PolygonizerWork LOD=%d Class=%s"), Chunk->LOD, *Name),
		Chunk->GetPriority())
	, LOD           (Chunk->LOD)
	, ChunkPosition (Chunk->Position)
	, MeshParameters(Chunk->Settings.bEnableTessellation, Chunk->Renderer->Settings.bOptimizeIndices)
	, VoxelSize     (Chunk->Renderer->Settings.VoxelSize)
	, Data          (Chunk->Renderer->Settings.Data.ToSharedRef())
	, DebugManager  (Chunk->Renderer->Settings.DebugManager)
	, UVConfig      (Chunk->Renderer->Settings.UVConfig)
	, UVScale       (Chunk->Renderer->Settings.UVScale)
	, NormalConfig  (Chunk->Renderer->Settings.NormalConfig)
	, MaterialConfig(Chunk->Renderer->Settings.MaterialConfig)
	, RenderChunk(Chunk->AsShared())
	, TaskId(Chunk->GetTaskId())
{
	check(IsInGameThread());
	Stats.StartStat("Waiting for thread queue", false);
}

FVoxelPolygonizerAsyncWorkBase::~FVoxelPolygonizerAsyncWorkBase()
{
	if (bDoWorkWasCalled)
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
	bDoWorkWasCalled = true;
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

	Stats.StartStat("Waiting for game thread", false);
}

void FVoxelPolygonizerAsyncWorkBase::PostDoWork()
{
	AsyncTask(ENamedThreads::GameThread, [Chunk = RenderChunk, TaskId = TaskId, bIsTransitionsTask = IsTransitionTask()]()
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

FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk, const FString& Name)
	: FVoxelPolygonizerAsyncWorkBase(Chunk, Name)
	, TransitionsMask(Chunk->GetWantedTransitionsMask())
{

}
