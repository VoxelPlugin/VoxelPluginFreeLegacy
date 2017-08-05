#pragma once
#include <cmath>
#include "CoreMinimal.h"
#include "Engine.h"

class AVoxelChunk;
class AVoxelWorld;

DECLARE_LOG_CATEGORY_EXTERN(ChunkOctreeLog, Log, All);

class ChunkOctree : public TSharedFromThis<ChunkOctree>
{
public:
	ChunkOctree(FIntVector position, int depth);
	~ChunkOctree();

	bool operator==(const ChunkOctree& other);

	// Center of the octree
	const FIntVector Position;

	const int Depth;



	int GetWidth();

	void CreateTree(AVoxelWorld* world, FVector cameraPosition);

	void Update(bool async);

	TWeakPtr<ChunkOctree> GetChunk(FIntVector position);

private:
	/*
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x

	*/
	TSharedPtr<ChunkOctree> Childs[8];

	bool bHasChilds;
	bool bHasChunk;

	AVoxelChunk* VoxelChunk;

	void Load(AVoxelWorld* world);
	void Unload();

	void CreateChilds();
	void DeleteChilds();
};