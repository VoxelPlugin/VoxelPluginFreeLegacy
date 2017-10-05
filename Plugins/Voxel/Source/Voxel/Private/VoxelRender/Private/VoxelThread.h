// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGrassType.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

class FVoxelPolygonizer;
class UVoxelChunk;
class FVoxelData;

/**
 * Thread to create foliage
 */
class FAsyncFoliageTask : public FNonAbandonableTask
{
public:
	FVoxelGrassVariety GrassVariety;

	// Output
	FStaticMeshInstanceData InstanceBuffer;
	TArray<FClusterNode> ClusterTree;
	int OutOcclusionLayerNum;

	FAsyncFoliageTask(FProcMeshSection Section, FVoxelGrassVariety GrassVariety, uint8 Material, float VoxelSize, FIntVector ChunkPosition, int Seed, UVoxelChunk* Chunk);

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncFoliageTask, STATGROUP_ThreadPoolAsyncTasks);
	};


private:
	UVoxelChunk* Chunk;

	FProcMeshSection const Section;
	uint8 const Material;
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
	FVoxelPolygonizer* Builder;
	UVoxelChunk* Chunk;

	FAsyncPolygonizerTask(FVoxelPolygonizer* InBuilder, UVoxelChunk* Chunk);
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncPolygonizerTask, STATGROUP_ThreadPoolAsyncTasks);
	};
};