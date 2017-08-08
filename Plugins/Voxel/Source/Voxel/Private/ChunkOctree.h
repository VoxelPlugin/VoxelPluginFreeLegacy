#pragma once
#include "CoreMinimal.h"

class AVoxelChunk;
class AVoxelWorld;

class ChunkOctree : public TSharedFromThis<ChunkOctree>
{
public:
	ChunkOctree(FIntVector Position, int Depth);

	bool operator==(const ChunkOctree& Other);

	// Center of the octree
	const FIntVector Position;

	const int Depth;

	void Delete();

	int Width();

	void CreateTree(AVoxelWorld* World, FVector CameraPosition);

	void Update(bool bAsync);

	TWeakPtr<ChunkOctree> GetChunk(FIntVector Position);

	AVoxelChunk* GetVoxelChunk();

private:
	/*
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x

	*/
	TArray<TSharedPtr<ChunkOctree>, TFixedAllocator<8>> Childs;

	bool bHasChilds;
	bool bHasChunk;

	AVoxelChunk* VoxelChunk;

	void Load(AVoxelWorld* World);
	void Unload();

	void CreateChilds();
	void DeleteChilds();
};