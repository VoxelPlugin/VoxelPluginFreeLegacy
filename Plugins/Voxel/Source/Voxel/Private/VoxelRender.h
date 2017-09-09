#pragma once

#include "CoreMinimal.h"
#include "ValueOctree.h"
#include "ProceduralMeshComponent.h"

class VoxelRender
{
public:
	VoxelRender(int Depth, ValueOctree* Octree, FIntVector ChunkPosition);

	void CreateSection(FProcMeshSection& OutSection);

private:
	const int Depth;
	const FIntVector ChunkPosition;

	uint64 Signs[125];

	// Caches to get index of already created vertices
	int Cache1[16][16][3];
	int Cache2[16][16][3];
	// Allows to ping-pong between 2 caches
	bool bNewCacheIs1;

	ValueOctree* Octree;

	FORCEINLINE int Width();
	// Step between cubes
	FORCEINLINE int Step();

	FColor GetMajorColor(FIntVector LowerCorner, uint32 CellWidth);

	void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index) override;
	int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex) override;

	void InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha);
};