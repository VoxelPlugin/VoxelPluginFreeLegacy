#pragma once

#include "CoreMinimal.h"
#include "VoxelProceduralMeshComponent.h"

#define CHUNKSIZE_FC 18

// Return the smallest multiple N of y such that:
//   x <= y * N
#define CEILING(x,y) (((x) + (y) - 1) / (y))

// A cube is 4x4x4 values, with last positions overlapping with next one ("owning" 3x3x3 values)
#define CUBE_COUNT_FC (CEILING(CHUNKSIZE_FC, 3))
#define END_CUBE_OFFSET_FC ((3 - CHUNKSIZE_FC % 3) % 3)

class FVoxelData;
struct FVoxelMaterial;

class FVoxelPolygonizerForCollisions
{
public:
	FVoxelPolygonizerForCollisions(FVoxelData* Data, const FIntVector& ChunkPosition, bool bEnableRender);

	bool CreateSection(FVoxelProcMeshSection& OutSection);

private:
	FVoxelData* const Data;
	FIntVector const ChunkPosition;
	const bool bEnableRender;


	// Cache of the sign of the values. Can lead to crash if value changed between cache and 2nd access
	uint64 CachedSigns[CUBE_COUNT_FC * CUBE_COUNT_FC * CUBE_COUNT_FC];

	// Cache to get index of already created vertices
	int Cache[CHUNKSIZE_FC][CHUNKSIZE_FC][CHUNKSIZE_FC][3];

	float CachedValues[(CHUNKSIZE_FC + 1) * (CHUNKSIZE_FC + 1) * (CHUNKSIZE_FC + 1)];

	FORCEINLINE float GetValue(int X, int Y, int Z);

	FORCEINLINE void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex);
};