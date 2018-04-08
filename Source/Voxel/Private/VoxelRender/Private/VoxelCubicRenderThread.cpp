// Copyright 2017 Phyronnaz

#include "VoxelCubicRenderThread.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "VoxelCrashReporter.h"
#include "Async.h"
#include "VoxelCubicPolygonizer.h"

DECLARE_CYCLE_STAT(TEXT("FAsyncCubicPolygonizerWork::DoWork"), STAT_FAsyncCubicPolygonizerWork_DoWork, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncCubicPolygonizerWork::DoWork.Mesh"), STAT_FAsyncCubicPolygonizerWork_DoWork_Mesh, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncCubicPolygonizerWork::DoWork.Grass"), STAT_FAsyncCubicPolygonizerWork_DoWork_Grass, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncCubicPolygonizerWork::DoWork.Actors"), STAT_FAsyncCubicPolygonizerWork_DoWork_Actors, STATGROUP_Voxel);

FAsyncCubicPolygonizerWork::FAsyncCubicPolygonizerWork(
	FVoxelData* Data,
	const FIntVector& ChunkPosition,
	const FIntVector& PositionOffset,
	AVoxelWorld* World
	)
	: Data(Data)
	, ChunkPosition(ChunkPosition)
	, PositionOffset(PositionOffset)
	, World(World)
	, IsDoneCounter(0)
{
}

void FAsyncCubicPolygonizerWork::DoWork()
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncCubicPolygonizerWork_DoWork, VOXEL_MULTITHREAD_STAT);

	// Mesh
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncCubicPolygonizerWork_DoWork_Mesh, VOXEL_MULTITHREAD_STAT);

		TSharedPtr<FVoxelCubicPolygonizer> Builder = MakeShareable(new FVoxelCubicPolygonizer(Data, ChunkPosition));

		bool bSuccess = Builder->CreateSection(Section);
		if (!bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
			Section.Reset();
		}
		for (auto& Vertex : Section.ProcVertexBuffer)
		{
			Vertex.Position += (FVector)PositionOffset;
		}
	}
	
}


int FAsyncCubicPolygonizerWork::GetPriority() const
{
	return 0;
}