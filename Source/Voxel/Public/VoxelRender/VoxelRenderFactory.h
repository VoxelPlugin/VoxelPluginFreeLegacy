// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"

class AVoxelWorld;
class AActor;
class IVoxelRender;
class FVoxelPolygonizerAsyncWork;
class FVoxelData;
class FVoxelQueuedThreadPool;
struct FIntBox;

class VOXEL_API FVoxelRenderFactory
{
public:
	static TSharedPtr<IVoxelRender> GetVoxelRender(EVoxelRenderType RenderType, AVoxelWorld* World);
	static TSharedPtr<FVoxelPolygonizerAsyncWork> GetAsyncWork(EVoxelRenderType RenderType, int LOD, const FIntVector& ChunkPosition, AVoxelWorld* World, TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, bool bComputeGrass, bool bComputeActors);
	static void CreatePolygonizerThreads(TArray<TSharedPtr<FVoxelPolygonizerAsyncWork>>& Threads, FVoxelQueuedThreadPool* ThreadPool, AVoxelWorld* World, TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, const FIntBox& Bounds, int ChunkLODOffset = 0, bool bCreateGrass = false);
};