// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGrassType.h"
#include "VoxelProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

class FVoxelPolygonizer;
class UVoxelChunkComponent;
class FVoxelData;
class UVoxelWorldGenerator;

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

	FAsyncFoliageTask(FVoxelProcMeshSection Section, FVoxelGrassVariety GrassVariety, int GrassVarietyIndex, uint8 Material, AVoxelWorld* World, FIntVector ChunkPosition, UVoxelChunkComponent* Chunk);

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncFoliageTask, STATGROUP_ThreadPoolAsyncTasks);
	};


private:
	UVoxelChunkComponent* Chunk;

	FVoxelProcMeshSection const Section;
	int GrassVarietyIndex;
	uint8 const Material;
	float const VoxelSize;
	FIntVector const ChunkPosition;
	int const Seed;
	UVoxelWorldGenerator* const Generator;
	AVoxelWorld* const World;
};


/**
 * Thread to create mesh
 */
class FAsyncPolygonizerTask : public FNonAbandonableTask
{
public:
	FVoxelPolygonizer* Builder;
	UVoxelChunkComponent* Chunk;

	FAsyncPolygonizerTask(FVoxelPolygonizer* InBuilder, UVoxelChunkComponent* Chunk);
	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncPolygonizerTask, STATGROUP_ThreadPoolAsyncTasks);
	};
};