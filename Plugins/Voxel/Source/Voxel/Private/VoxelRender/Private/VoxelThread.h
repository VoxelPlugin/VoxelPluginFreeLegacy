// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

class VoxelPolygonizer;
class AVoxelChunk;
class VoxelData;

/**
 * Thread to create foliage
 */
class FAsyncFoliageTask : public FNonAbandonableTask
{
public:
	FGrassVariety GrassVariety;

	// Output
	FStaticMeshInstanceData InstanceBuffer;
	TArray<FClusterNode> ClusterTree;
	int OutOcclusionLayerNum;

	FAsyncFoliageTask(FProcMeshSection& Section, FGrassVariety GrassVariety, uint8 Material, float VoxelSize, FIntVector ChunkPosition, int Seed, AVoxelChunk* Chunk);

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncFoliageTask, STATGROUP_ThreadPoolAsyncTasks);
	};


private:
	AVoxelChunk* Chunk;

	FProcMeshSection const Section;
	uint8 const Material;
	FTransform const ChunkTransform;
	float const VoxelSize;
	FIntVector const ChunkPosition;
	int const Seed;
};


/**
 * Thread to create mesh
 */
class FAsyncPolygonizerTask : public FNonAbandonableTask
{
public:
	VoxelPolygonizer* Builder;
	AVoxelChunk* Chunk;

	FAsyncPolygonizerTask(VoxelPolygonizer* InBuilder, AVoxelChunk* Chunk);
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncPolygonizerTask, STATGROUP_ThreadPoolAsyncTasks);
	};
};