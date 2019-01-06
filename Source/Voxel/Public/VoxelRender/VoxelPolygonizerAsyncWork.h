// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelThreadPool.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelConfigEnums.h"
#include "VoxelDebug/VoxelStats.h"

class AVoxelWorld;
class FVoxelData;
class FVoxelRenderChunk;
struct FVoxelGrassBuffer;

class VOXEL_API FVoxelPolygonizerAsyncWorkBase : public FVoxelAsyncWork
{
public:
	const int LOD;
	const uint64 Distance;
	const FIntVector ChunkPosition;
	TSharedRef<FVoxelData, ESPMode::ThreadSafe> const Data;
	const int VoxelSize;
	const EVoxelNormalConfig NormalConfig;
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelUVConfig UVConfig;
	const FVoxelMeshProcessingParameters MeshParameters;
	// Output
	TSharedRef<FVoxelChunk> Chunk = MakeShared<FVoxelChunk>();

	FVoxelPolygonizerAsyncWorkBase(
		int LOD, 
		uint64 Distance,
		const FIntVector& ChunkPosition,
		AVoxelWorld* World, 
		TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data, // Data in constructor as sometimes World.Data != Data (eg asset actor)
		FVoxelRenderChunk* ChunkCallback);

	virtual ~FVoxelPolygonizerAsyncWorkBase() override
	{
		FVoxelStats::AddElement(Stats);
	}

	virtual uint64 GetPriority() const final { return MAX_uint64 - Distance; }

protected:
	FVoxelStatsElement Stats;

	void ShowError();
};

typedef int FVoxelPreviousGrassInfo;

class VOXEL_API FVoxelPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:

	FVoxelPolygonizerAsyncWork(
		int LOD,
		uint64 Distance,
		const FIntVector& ChunkPosition,
		AVoxelWorld* World,
		TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data,
		bool bComputeGrass,
		bool bComputeActors,
		const FVoxelPreviousGrassInfo& OldPreviousGrassInfo = FVoxelPreviousGrassInfo(),
		FVoxelRenderChunk* ChunkCallback = nullptr);
	
};

class VOXEL_API FVoxelTransitionsPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWorkBase
{
public:
	const uint8 TransitionsMask;
	
	FVoxelTransitionsPolygonizerAsyncWork(
		int LOD,
		uint64 Distance,
		const FIntVector& ChunkPosition,
		AVoxelWorld* World,
		TSharedRef<FVoxelData, ESPMode::ThreadSafe> Data,
		FVoxelRenderChunk* ChunkCallback,
		uint8 TransitionsMask)
		: FVoxelPolygonizerAsyncWorkBase(LOD, Distance, ChunkPosition, World, Data, ChunkCallback)
		, TransitionsMask(TransitionsMask)
	{
	}
};
