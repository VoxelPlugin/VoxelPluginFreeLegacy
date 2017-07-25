#pragma once
#include <cmath>
#include "CoreMinimal.h"
#include "Engine.h"

class AVoxelChunk;
class AVoxelWorld;

class ChunkOctree
{
public:
	ChunkOctree(int x, int y, int z, int depth);

	inline bool operator==(ChunkOctree* other);


	const int X;
	const int Y;
	const int Z;
	const int Depth;



	inline int GetWidth();

	void CreateTree(AVoxelWorld* world, FVector cameraPosition);

	void Update();

	ChunkOctree* GetLeaf(int x, int y, int z);

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