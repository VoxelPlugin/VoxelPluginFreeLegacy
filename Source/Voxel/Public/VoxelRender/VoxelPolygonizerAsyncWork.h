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
	const int32 LOD;
	const FIntVector ChunkPosition;
	const FVoxelMeshProcessingParameters MeshParameters;

	const int32 VoxelSize;
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> const Data;
	TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe> const DebugManager;
	const EVoxelUVConfig UVConfig;
	const float UVScale;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;

	// Output
	TSharedRef<FVoxelChunk> Chunk = MakeShared<FVoxelChunk>();

	FVoxelStatsElement Stats;

	FVoxelPolygonizerAsyncWorkBase(FVoxelRenderChunk* Chunk, const FString& Name);

	virtual ~FVoxelPolygonizerAsyncWorkBase() override;

protected:
	virtual TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() = 0;
	virtual EVoxelStatsType GetTaskType() = 0;
	virtual bool IsTransitionTask() = 0;

private:
	void DoWork() final;
	void PostDoWork() final;

	const TWeakPtr<FVoxelRenderChunk, ESPMode::ThreadSafe> RenderChunk;
	const uint64 TaskId;
	bool bDoWorkWasCalled = false;
};

class VOXEL_API FVoxelPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:
	using FVoxelPolygonizerAsyncWorkBase::FVoxelPolygonizerAsyncWorkBase;
	
protected:
	virtual bool IsTransitionTask() final { return false; }
};

class VOXEL_API FVoxelTransitionsPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:
	const uint8 TransitionsMask;
	
	FVoxelTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk, const FString& Name);

protected:
	virtual bool IsTransitionTask() final { return true; }
};