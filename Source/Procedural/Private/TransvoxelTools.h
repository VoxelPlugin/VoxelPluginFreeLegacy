#pragma once

#include<forward_list>
#include "CoreMinimal.h"

enum TransitionDirection { XMin, XMax, YMin, YMax, ZMin, ZMax };

struct FBoolVector
{
	bool X;
	bool Y;
	bool Z;

	FBoolVector(bool x, bool y, bool z) : X(x), Y(y), Z(z) {}
};


struct VertexProperties
{
	bool IsNearXMin;
	bool IsNearXMax;

	bool IsNearYMin;
	bool IsNearYMax;

	bool IsNearZMin;
	bool IsNearZMax;

	bool IsNormalOnly;
};

struct VertexProperties2D
{
	int X;
	int Y;

	bool IsXExact;
	bool IsYExact;

	bool NeedTranslation;

	TransitionDirection Direction;
};

struct IRegularVoxel
{
	virtual ~IRegularVoxel() {}
	virtual signed char GetValue(int x, int y, int z) = 0;
	virtual FColor GetColor(int x, int y, int z) = 0;
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index, VertexProperties properties) = 0;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) = 0;
	virtual int GetDepth() = 0;
	virtual bool IsNormalOnly(FVector vertex) = 0;
};

struct ITransitionVoxel
{
	virtual ~ITransitionVoxel() {}
	virtual signed char GetValue2D(int x, int y) = 0;
	virtual FColor GetColor2D(int x, int y) = 0;
	virtual void SaveVertex2D(int x, int y, short edgeIndex, int index) = 0;
	virtual int LoadVertex2D(int x, int y, short direction, short edgeIndex) = 0;
	virtual int GetDepth() = 0;
};

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<FColor> Colors;
typedef std::forward_list<VertexProperties> Props;
typedef std::forward_list<VertexProperties2D> Props2D;


namespace TransvoxelTools
{
	void RegularPolygonize(IRegularVoxel* chunk, int x, int y, int z, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props& properties, Colors& colors, int& verticesCount);
	int AddVertex(IRegularVoxel* chunk, int step, Verts& vertices, Props& properties, Colors& colors, int& verticesCount, FVector vertex, FIntVector exactPosition, FBoolVector isExact);

	FVector InterpolateX(IRegularVoxel* chunk, int xMin, int xMax, int y, int z);
	FVector InterpolateY(IRegularVoxel* chunk, int x, int yMin, int yMax, int z);
	FVector InterpolateZ(IRegularVoxel* chunk, int x, int y, int zMin, int zMax);

	void TransitionPolygonize(ITransitionVoxel* chunk, int x, int y, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props2D& properties, Colors& colors, int& verticesCount, TransitionDirection direction);
	int AddVertex(ITransitionVoxel* chunk, bool isTranslated, int step, Verts& vertices, Props2D& properties, Colors& colors, int& verticesCount, FVector vertex, FIntVector exactPosition, FBoolVector isExact, TransitionDirection direction);

	FVector InterpolateX2D(ITransitionVoxel* chunk, int xMin, int xMax, int y);
	FVector InterpolateY2D(ITransitionVoxel* chunk, int x, int yMin, int yMax);

	FVector GetTranslated(FVector V, FVector N, VertexProperties P, const bool ChunkHasHigherRes[6], int depth);
};