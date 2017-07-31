#pragma once

#include<forward_list>
#include "CoreMinimal.h"
#include "VertexProperties.h"

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<VertexProperties> Props;
typedef std::forward_list<VertexProperties2D> Props2D;

struct FBoolVector
{
	bool X;
	bool Y;
	bool Z;

	FBoolVector(bool x, bool y, bool z) : X(x), Y(y), Z(z) {}
};

class IRegularVoxel
{
public:
	virtual ~IRegularVoxel() {}
	virtual signed char GetValue(int x, int y, int z) = 0;
	virtual void SaveVertex(int x, int y, int z, short edgeIndex, int index) = 0;
	virtual int LoadVertex(int x, int y, int z, short direction, short edgeIndex) = 0;
	virtual int GetDepth() = 0;
	virtual bool IsNormalOnly(FVector vertex) = 0;
};

class ITransitionVoxel
{
public:
	virtual ~ITransitionVoxel() {}
	virtual signed char GetValue(int x, int y) = 0;
	virtual void SaveVertex(int x, int y, short edgeIndex, int index) = 0;
	virtual int LoadVertex(int x, int y, short direction, short edgeIndex) = 0;
	virtual int GetDepth() = 0;
	virtual FIntVector GetRealPosition(int x, int y) = 0;
	virtual FBoolVector GetRealIsExact(bool xIsExact, bool yIsExact) = 0;
};

namespace TransvoxelTools
{
	void RegularPolygonize(IRegularVoxel* chunk, int x, int y, int z, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props& properties, int& verticesCount);
	int AddVertex(IRegularVoxel* chunk, int step, Verts& vertices, Props& properties, int& verticesCount, FVector vertex, FIntVector exactPosition, bool xIsExact = true, bool yIsExact = true, bool zIsExact = true);

	FVector InterpolateX(IRegularVoxel* chunk, int xMin, int xMax, int y, int z);
	FVector InterpolateY(IRegularVoxel* chunk, int x, int yMin, int yMax, int z);
	FVector InterpolateZ(IRegularVoxel* chunk, int x, int y, int zMin, int zMax);

	void TransitionPolygonize(ITransitionVoxel* chunk, int x, int y, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props2D& properties, int& verticesCount);
	int AddVertex(ITransitionVoxel* chunk, bool isTranslated, int step, Verts& vertices, Props2D& properties, int& verticesCount, FVector vertex, FIntVector exactPosition, FBoolVector IsExact);

	FVector InterpolateX(ITransitionVoxel* chunk, int xMin, int xMax, int y);
	FVector InterpolateY(ITransitionVoxel* chunk, int x, int yMin, int yMax);
};

