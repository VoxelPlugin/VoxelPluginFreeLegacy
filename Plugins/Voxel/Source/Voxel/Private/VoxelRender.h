#pragma once

#include "CoreMinimal.h"
#include "VoxelData.h"
#include "ProceduralMeshComponent.h"

class VoxelRender
{
public:
	VoxelRender(int Depth, VoxelData* Data, FIntVector ChunkPosition);

	void CreateSection(FProcMeshSection& OutSection);

private:
	const int Depth;
	const FIntVector ChunkPosition;

	uint64 Signs[216];

	// Cache to get index of already created vertices
	int Cache[17][17][17][3];

	VoxelData* Data;

	FORCEINLINE int Width();
	// Step between cubes
	FORCEINLINE int Step();

	FColor GetMajorColor(FIntVector LowerCorner, uint32 CellWidth);

	 void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index);
	 int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex);

	void InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha);
};