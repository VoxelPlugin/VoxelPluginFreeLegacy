#pragma once
#include <cmath>
#include "CoreMinimal.h"
#include "Engine.h"

class AVoxelChunk;
class AVoxelWorld;

class ChunkOctree
{
public:
	ChunkOctree(FIntVector position, int depth);

	bool operator==(ChunkOctree* other);

	// Center of the octree
	const FIntVector Position;
	const int Depth;



	int GetWidth();

	void CreateTree(AVoxelWorld* world, FVector cameraPosition);

	void Update();

	ChunkOctree* GetLeaf(FIntVector position);

	bool IsLeaf();

protected:
	/*
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x

	*/
	ChunkOctree* Childs[8];

	AVoxelChunk* VoxelChunk = nullptr;

	bool CreateChilds();
};