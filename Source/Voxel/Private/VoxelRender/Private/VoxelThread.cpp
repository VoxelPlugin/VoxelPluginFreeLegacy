// Copyright 2018 Phyronnaz

#include "VoxelThread.h"
#include "VoxelPrivate.h"
#include "VoxelData.h"
#include "VoxelWorld.h"
#include "VoxelWorldGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "VoxelCrashReporter.h"
#include "Async.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelAsyncWork::WaitForCompletion"), STAT_FVoxelAsyncWork_WaitForCompletion, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork"), STAT_FAsyncPolygonizerWork_DoWork, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Mesh"), STAT_FAsyncPolygonizerWork_DoWork_Mesh, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Grass"), STAT_FAsyncPolygonizerWork_DoWork_Grass, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWork::DoWork.Actors"), STAT_FAsyncPolygonizerWork_DoWork_Actors, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("FAsyncPolygonizerWorkForTransitions::DoWork"), STAT_FAsyncPolygonizerWorkForTransitions_DoWork, STATGROUP_Voxel);

FVoxelAsyncWork::FVoxelAsyncWork()
{
	DoneEvent = FPlatformProcess::GetSynchEventFromPool(true);
	DoneEvent->Reset();
}

FVoxelAsyncWork::~FVoxelAsyncWork()
{
	if (DoneEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
		DoneEvent = nullptr;
	}
}

void FVoxelAsyncWork::DoThreadedWork()
{
	DoWork();

	{
		FScopeLock Lock(&DoneSection);
		DoneEvent->Trigger();
		IsDoneCounter.Increment();
	}
}

void FVoxelAsyncWork::Abandon()
{
	FScopeLock Lock(&DoneSection);

	DoneEvent->Trigger();
	IsDoneCounter.Increment();
}

bool FVoxelAsyncWork::IsDone() const
{
	return IsDoneCounter.GetValue() > 0;
}

void FVoxelAsyncWork::WaitForCompletion()
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelAsyncWork_WaitForCompletion);

	DoneEvent->Wait();
	// Make sure IsDoneCounter had the time to increment
	FScopeLock Lock(&DoneSection);
}

///////////////////////////////////////////////////////////////////////////////

FAsyncPolygonizerWork::FAsyncPolygonizerWork(
	int LOD,
	FVoxelData* Data,
	const FIntVector& ChunkPosition,
	const FIntVector& PositionOffset,
	AVoxelWorld* World
	)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, PositionOffset(PositionOffset)
	, World(World)
	, IsDoneCounter(0)
{
}

void FAsyncPolygonizerWork::DoWork()
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork, VOXEL_MULTITHREAD_STAT);

	// Mesh
	{
		CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWork_DoWork_Mesh, VOXEL_MULTITHREAD_STAT);

		TSharedPtr<FVoxelPolygonizer> Builder = MakeShareable(new FVoxelPolygonizer(LOD, Data, ChunkPosition, World ? World->GetCreateAdditionalVerticesForMaterialsTransitions() : true, World ? World->GetEnableNormals() : true));

		bool bSuccess = Builder->CreateChunk(Chunk);
		if (!bSuccess)
		{
			AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
			Chunk.Reset();
		}
		for (auto& Vertex : Chunk.VertexBuffer)
		{
			Vertex.Position += (FVector)PositionOffset;
		}
	}
	
}


int FAsyncPolygonizerWork::GetPriority() const
{
	return -LOD;
}

///////////////////////////////////////////////////////////////////////////////

FAsyncPolygonizerForTransitionsWork::FAsyncPolygonizerForTransitionsWork(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, uint8 TransitionsMask)
	: LOD(LOD)
	, Data(Data)
	, ChunkPosition(ChunkPosition)
	, TransitionsMask(TransitionsMask)
{

}

void FAsyncPolygonizerForTransitionsWork::DoWork()
{
	CONDITIONAL_SCOPE_CYCLE_COUNTER(STAT_FAsyncPolygonizerWorkForTransitions_DoWork, VOXEL_MULTITHREAD_STAT);

	TSharedPtr<FVoxelPolygonizerForTransitions> Builder = MakeShareable(new FVoxelPolygonizerForTransitions(LOD, Data, ChunkPosition));

	bool bSuccess = Builder->CreateTransitions(VertexBuffer, IndexBuffer, TransitionsMask);
	if (!bSuccess)
	{
		AsyncTask(ENamedThreads::GameThread, []() { FVoxelCrashReporter::ShowApproximationError(); });
		VertexBuffer.Reset();
		IndexBuffer.Reset();
	}
}

int FAsyncPolygonizerForTransitionsWork::GetPriority() const
{
	return -LOD;
}
