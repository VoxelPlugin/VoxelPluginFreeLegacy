#pragma once

#include "TransvoxelTools.h"

class AVoxelChunk;

struct VoxelChunkStruct : IRegularVoxel, ITransitionVoxel
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
	float GetValue(int X, int Y, int Z) override;
	FColor GetColor(int X, int Y, int Z) override;
	void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index, VertexProperties Properties) override;
	int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex) override;
	bool IsNormalOnly(FVector Vertex) override;

	// Inherited via ITransitionVoxel
	float GetValue2D(int X, int Y) override;
	FColor GetColor2D(int X, int Y) override;
	void SaveVertex2D(int X, int Y, short EdgeIndex, int Index) override;
	int LoadVertex2D(int X, int Y, short Direction, short EdgeIndex) override;
	TransitionDirection GetDirection() override;


	float Values[19][19][19];

	TransitionDirection CurrentDirection;

	// Local to global
	FIntVector TransformPosition(int X, int Y, int Z, TransitionDirection Direction) const;
	FVector TransformPosition(FVector Vertex, TransitionDirection Direction) const;
	FBoolVector TransformPosition(FBoolVector Vertex, TransitionDirection Direction) const;
	// Global to local
	FIntVector InverseTransformPosition(int X, int Y, int Z, TransitionDirection Direction) const;

	int Cache1[18][18][4];
	int Cache2[18][18][4];
	bool bNewCacheIs1;

	int Cache2D[6][16][16][10];

	AVoxelChunk* Chunk;
};