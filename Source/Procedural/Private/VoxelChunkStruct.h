#pragma once

#include "TransvoxelTools.h"

class AVoxelChunk;

struct VoxelChunkStruct : public IRegularVoxel, public ITransitionVoxel
{
	VoxelChunkStruct(AVoxelChunk* chunk);


	bool ChunkHasHigherRes[6];

	const int Depth;
	// List of pair of vertex index that are at the same position. Top: From; Bottom: To
	//std::forward_list<int> EquivalenceList;

	// Inherited via IRegularVoxel
	virtual signed char GetValue(int x, int y, int z) override;
	virtual FColor GetColor(int x, int y, int z) override;
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index, VertexProperties properties) override;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) override;
	virtual int GetDepth() override;
	virtual bool IsNormalOnly(FVector vertex) override;

	// Inherited via ITransitionVoxel
	virtual signed char GetValue2D(int x, int y) override;
	virtual FColor GetColor2D(int x, int y) override;
	virtual void SaveVertex2D(int x, int y, short edgeIndex, int index) override;
	virtual int LoadVertex2D(int x, int y, short direction, short edgeIndex) override;


	signed char Values[19][19][19];

	TransitionDirection CurrentDirection;

	// Local to global
	FIntVector TransformPosition(int x, int y, int z, TransitionDirection direction);
	FVector TransformPosition(FVector vertex, TransitionDirection direction);
	FBoolVector TransformPosition(FBoolVector vertex, TransitionDirection direction);
	// Global to local
	FIntVector InverseTransformPosition(int x, int y, int z, TransitionDirection direction);

	int Cache1[18][18][4];
	int Cache2[18][18][4];
	bool NewCacheIs1;

	int Cache2D[6][16][16][10];

	AVoxelChunk* Chunk;
};