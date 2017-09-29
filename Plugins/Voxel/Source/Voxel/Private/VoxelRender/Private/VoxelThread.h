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
 * Thread to create mesh section
 */
class MeshBuilderAsyncTask : public IQueuedWork
{
public:
	MeshBuilderAsyncTask(uint8 Depth, VoxelData* Data, FIntVector Position, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, bool bComputeTransitions, AVoxelChunk* Chunk);
	virtual ~MeshBuilderAsyncTask() override;

	virtual void DoThreadedWork() override;

	// Do not call
	virtual void Abandon() override;

	FProcMeshSection& GetSection();

private:
	AVoxelChunk* Chunk;
	bool bComputeTransitions;
	FProcMeshSection Section;
	VoxelPolygonizer* Worker;
};


/**
 * Thread to create foliage
 */
class FoliageBuilderAsyncTask : public IQueuedWork
{
public:
	FGrassVariety GrassVariety;

	// Output
	FStaticMeshInstanceData InstanceBuffer;
	TArray<FClusterNode> ClusterTree;
	int OutOcclusionLayerNum;

	FoliageBuilderAsyncTask(FProcMeshSection Section, FGrassVariety GrassVariety, uint8 Material, FTransform ChunkTransform, float VoxelSize, FIntVector ChunkPosition, int Seed, AVoxelChunk* Chunk);

	virtual void DoThreadedWork() override;

	// Do not call
	virtual void Abandon() override;

private:
	AVoxelChunk* Chunk;

	FProcMeshSection const Section;
	uint8 const Material;
	FTransform const ChunkTransform;
	float const VoxelSize;
	FIntVector const ChunkPosition;
	int const Seed;
};
