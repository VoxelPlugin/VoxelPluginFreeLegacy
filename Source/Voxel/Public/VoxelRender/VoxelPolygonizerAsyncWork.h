// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelThreadPool.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelConfigEnums.h"
#include "VoxelDebug/VoxelStats.h"

class FVoxelData;
class FVoxelRenderChunk;
struct FVoxelRendererSettings;
class FVoxelDebugManager;
class FVoxelPolygonizerBase;

class VOXEL_API FVoxelPolygonizerAsyncWorkBase : public FVoxelAsyncWork
{
public:
	const int LOD;
	const FIntVector ChunkPosition;
	const FVoxelMeshProcessingParameters MeshParameters;

	const int VoxelSize;
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> const Data;
	TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe> const DebugManager;
	const EVoxelUVConfig UVConfig;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;

	// Output
	TSharedRef<FVoxelChunk> Chunk = MakeShared<FVoxelChunk>();

	FVoxelStatsElement Stats;

	FVoxelPolygonizerAsyncWorkBase(FVoxelRenderChunk* Chunk, bool bIsTransitionsTask);

	virtual ~FVoxelPolygonizerAsyncWorkBase() override;

protected:
	virtual void PostMeshCreation() {};
	virtual TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() = 0;
	virtual EVoxelStatsType GetTaskType() = 0;

private:
	void DoWork() final;
	static void Callback(TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> Chunk, bool bIsTransitionsTask, uint64 TaskId);
};

typedef int FVoxelPreviousGrassInfo;

class VOXEL_API FVoxelPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:

	FVoxelPolygonizerAsyncWork(FVoxelRenderChunk* Chunk);
	
};

class VOXEL_API FVoxelTransitionsPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:
	const uint8 TransitionsMask;
	
	FVoxelTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk);
};