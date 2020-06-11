// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelPriorityHandler.h"
#include "VoxelAsyncWork.h"

struct FVoxelRendererSettings;
struct FVoxelChunkMesh;
class FVoxelDefaultRenderer;
class FVoxelMesherBase;

class VOXEL_API FVoxelMesherAsyncWork : public FVoxelAsyncWork
{
public:
	const uint64 TaskId = UNIQUE_ID();
	
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
		uint8 TransitionsMask);

	static void CreateGeometry_AnyThread(
		const FVoxelDefaultRenderer& Renderer,
		int32 LOD,
		const FIntVector& ChunkPosition,
		TArray<uint32>& OutIndices,
		TArray<FVector>& OutVertices);

private:
	// Important: do not allow public delete
	virtual ~FVoxelMesherAsyncWork() override;
	
	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() override final;
	virtual void PostDoWork() override final;
	virtual uint32 GetPriority() const override final;
	//~ End FVoxelAsyncWork Interface

	static TUniquePtr<FVoxelMesherBase> GetMesher(
		const FVoxelRendererSettings& Settings,
		int32 LOD,
		const FIntVector& ChunkPosition,
		bool bIsTransitionTask,
		uint8 TransitionsMask);
	
	
	const TVoxelWeakPtr<FVoxelDefaultRenderer> Renderer;
	const FVoxelPriorityHandler PriorityHandler;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
};