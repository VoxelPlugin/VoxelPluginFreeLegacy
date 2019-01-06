// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelLogStatDefinitions.h"
#include "Async/Async.h"
#include "Misc/ScopeLock.h"
#include "Renderers/VoxelRenderChunk.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"
#include "Misc/MessageDialog.h"

FVoxelPolygonizerAsyncWorkBase::FVoxelPolygonizerAsyncWorkBase(
	int LOD,
	uint64 Distance,
	const FIntVector& ChunkPosition,
	AVoxelWorld* World, 
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data,
	FVoxelRenderChunk* ChunkCallback)
	: FVoxelAsyncWork(ChunkCallback)
	, LOD(LOD)
	, Distance(Distance)
	, ChunkPosition(ChunkPosition)
	, Data(Data)
	, VoxelSize(World->GetVoxelSize())
	, NormalConfig(World->GetNormalConfig())
	, MaterialConfig(World->GetMaterialConfig())
	, UVConfig(World->GetUVConfig())
	, MeshParameters(World->IsTessellationEnabled(LOD), World->GetOptimizeIndices())
{
	Stats.SetLOD(LOD);
	Stats.StartStat("Waiting In Thread Queue");
}

#define LOCTEXT_NAMESPACE "ShowWorldGeneratorError"

void ShowWorldGeneratorError(FVoxelData* Data)
{
	static TSet<FVoxelData*> IgnoredDatas;
	if (!IgnoredDatas.Contains(Data))
	{
		auto Result = FMessageDialog::Open(
			EAppMsgType::YesNo,
			LOCTEXT("GraphError",
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

void FVoxelPolygonizerAsyncWorkBase::ShowError()
{
	auto* DataPtr = &Data.Get();
	AsyncTask(ENamedThreads::GameThread, [=]() { ShowWorldGeneratorError(DataPtr); });
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork(
	int LOD,
	uint64 Distance,
	const FIntVector& ChunkPosition,
	AVoxelWorld* World,
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data,
	bool bComputeGrass,
	bool bComputeActors,
	const FVoxelPreviousGrassInfo& OldPreviousGrassInfo,
	FVoxelRenderChunk* ChunkCallback)
	: FVoxelPolygonizerAsyncWorkBase(LOD, Distance, ChunkPosition, World, Data, ChunkCallback)
{
}

