// Copyright 2018 Phyronnaz

#include "VoxelRender/VoxelRenderFactory.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelWorld.h"
#include "Renderers/VoxelMCRenderer.h"
#include "Renderers/VoxelCubicRenderer.h"
#include "AsyncWorks/VoxelMCPolygonizerAsyncWork.h"
#include "AsyncWorks/VoxelCubicPolygonizerAsyncWork.h"
#include "VoxelData/VoxelData.h"

TSharedPtr<IVoxelRender> FVoxelRenderFactory::GetVoxelRender(EVoxelRenderType RenderType, AVoxelWorld* World)
{
	switch (RenderType)
	{
	case EVoxelRenderType::MarchingCubes:
		return MakeShared<FVoxelMCRenderer>(World);
	case EVoxelRenderType::Cubic:
		return MakeShared<FVoxelCubicRenderer>(World);
	default:
		check(false);
		return TSharedPtr<IVoxelRender>();
	}
}

TSharedPtr<FVoxelPolygonizerAsyncWork> FVoxelRenderFactory::GetAsyncWork(EVoxelRenderType RenderType, int LOD, const FIntVector& ChunkPosition, AVoxelWorld* World, TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, bool bComputeGrass, bool bComputeActors)
{
	switch (RenderType)
	{
	case EVoxelRenderType::MarchingCubes:
		return MakeShared<FVoxelMCPolygonizerAsyncWork>(LOD, 0, ChunkPosition, World, Data, bComputeGrass, bComputeActors);
	case EVoxelRenderType::Cubic:
		return MakeShared<FVoxelCubicPolygonizerAsyncWork>(LOD, 0, ChunkPosition, World, Data, bComputeGrass, bComputeActors);
	default:
		check(false);
		return TSharedPtr<FVoxelPolygonizerAsyncWork>();
	}
}

void FVoxelRenderFactory::CreatePolygonizerThreads(TArray<TSharedPtr<FVoxelPolygonizerAsyncWork>>& Threads, FVoxelQueuedThreadPool* ThreadPool, AVoxelWorld* World, TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, const FIntBox& Bounds, int ChunkLODOffset, bool bCreateGrass)
{
	const FIntVector Size(Data->Size() / 2);
	const int ChunkSize = CHUNK_SIZE << ChunkLODOffset;

	FIntVector Min = FVoxelIntVector::Max(FVoxelIntVector::FloorToInt(FVector(Bounds.Min) / ChunkSize) * ChunkSize, -Size);
	FIntVector Max = FVoxelIntVector::Min(FVoxelIntVector::CeilToInt(FVector(Bounds.Max) / ChunkSize) * ChunkSize, Size);

	for (int X = Min.X; X < Max.X; X += ChunkSize)
	{
		for (int Y = Min.Y; Y < Max.Y; Y += ChunkSize)
		{
			for (int Z = Min.Z; Z < Max.Z; Z += ChunkSize)
			{
				const FIntVector Position = FIntVector(X, Y, Z);

				TSharedPtr<FVoxelPolygonizerAsyncWork> Thread = GetAsyncWork(World->GetRenderType(), ChunkLODOffset, Position, World, Data, bCreateGrass, false);
				ThreadPool->AddQueuedWork(Thread.Get());
				Threads.Add(Thread);
			}
		}
	}
}
