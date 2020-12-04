// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelAsyncWork.h"

class FVoxelData;
class IVoxelRenderer;
class FVoxelMesherBase;
class FVoxelDefaultRenderer;
class FVoxelRuntimeSettings;
struct FVoxelChunkMesh;

class VOXEL_API FVoxelMesherAsyncWork : public FVoxelAsyncWork
{
	GENERATED_VOXEL_ASYNC_WORK_BODY(FVoxelMesherAsyncWork)
	
public:
	const uint64 TaskId = VOXEL_UNIQUE_ID();
	
	const uint64 ChunkId;
	const int32 LOD;
	const FIntVector ChunkPosition;
	const bool bIsTransitionTask;
	const uint8 TransitionsMask; // If bIsTransitionTask is true

	// Output
	TVoxelSharedPtr<FVoxelChunkMesh> Chunk;
	double CreationTime = 0;

	FVoxelMesherAsyncWork(
		FVoxelDefaultRenderer& Renderer,
		uint64 ChunkId,
		int32 LOD,
		const FVoxelIntBox& Bounds,
		bool bIsTransitionTask,
		uint8 TransitionsMask,
		EVoxelTaskType TaskType);

	static void CreateGeometry_AnyThread(
		const FIntVector& ChunkPosition,
		const IVoxelRenderer& Renderer,
		const FVoxelData& Data,
		int32 LOD,
		TArray<uint32>& OutIndices,
		TArray<FVector>& OutVertices);

private:
	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() override final;
	virtual void PostDoWork() override final;
	//~ End FVoxelAsyncWork Interface

	static TUniquePtr<FVoxelMesherBase> GetMesher(
		const FIntVector& ChunkPosition,
		const IVoxelRenderer& Renderer,
		const FVoxelData& Data,
		int32 LOD,
		bool bIsTransitionTask,
		uint8 TransitionsMask);
	
	const TVoxelWeakPtr<FVoxelDefaultRenderer> Renderer;
};