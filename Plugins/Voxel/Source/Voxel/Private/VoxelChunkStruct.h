#pragma once

#include "TransvoxelTools.h"

class AVoxelChunk;

struct VoxelChunkStruct : public IRegularVoxel, public ITransitionVoxel
{
	VoxelChunkStruct(AVoxelChunk* Chunk);


	bool ChunkHasHigherRes[6];

	const int Depth;
	const int Width;
	const int Step;

	// List of pair of vertex index that are at the same position. Top: From; Bottom: To
	std::forward_list<int> EquivalenceList;
	// Vertices with same normal
	std::forward_list<int> NormalsEquivalenceList;

	// Inherited via IRegularVoxel
	virtual signed char GetValue(int X, int Y, int Z) override;
	virtual FColor GetColor(int X, int Y, int Z) override;
	virtual void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index, VertexProperties Properties) override;
	virtual int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex) override;
	virtual bool IsNormalOnly(FVector Vertex) override;

	// Inherited via ITransitionVoxel
	virtual signed char GetValue2D(int X, int Y) override;
	virtual FColor GetColor2D(int X, int Y) override;
	virtual void SaveVertex2D(int X, int Y, short EdgeIndex, int Index) override;
	virtual int LoadVertex2D(int X, int Y, short Direction, short EdgeIndex) override;
	TransitionDirection GetDirection();


	signed char Values[19][19][19];

	TransitionDirection CurrentDirection;

	// Local to global
	FIntVector TransformPosition(int X, int Y, int Z, TransitionDirection Direction);
	FVector TransformPosition(FVector Vertex, TransitionDirection Direction);
	FBoolVector TransformPosition(FBoolVector Vertex, TransitionDirection Direction);
	// Global to local
	FIntVector InverseTransformPosition(int X, int Y, int Z, TransitionDirection Direction);

	int Cache1[18][18][4];
	int Cache2[18][18][4];
	bool NewCacheIs1;

	int Cache2D[6][16][16][10];

	AVoxelChunk* Chunk;
};