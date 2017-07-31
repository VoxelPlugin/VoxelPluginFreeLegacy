#pragma once

#include<forward_list>
#include "CoreMinimal.h"
#include "VertexProperties.h"

typedef std::forward_list<int> Trigs;
typedef std::forward_list<FVector> Verts;
typedef std::forward_list<VertexProperties> Props;

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

namespace TransvoxelTools
{
	void RegularPolygonize(IRegularVoxel* chunk, int x, int y, int z, short validityMask, Trigs& triangles, int& trianglesCount, Verts& vertices, Props& properties, int& verticesCount);

	int AddVertex(IRegularVoxel* chunk, int step, Verts& vertices, Props& properties, int& verticesCount, FVector vertex, FIntVector exactPosition, bool xIsExact = true, bool yIsExact = true, bool zIsExact = true);

	FVector InterpolateX(IRegularVoxel* chunk, int xMin, int xMax, int y, int z);
	FVector InterpolateY(IRegularVoxel* chunk, int x, int yMin, int yMax, int z);
	FVector InterpolateZ(IRegularVoxel* chunk, int x, int y, int zMin, int zMax);
};

