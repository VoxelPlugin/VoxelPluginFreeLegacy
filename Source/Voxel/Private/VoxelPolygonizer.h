// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDirection.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelProceduralMeshComponent.h"

// Return the smallest multiple N of y such that:
//   x <= y * N
#define CEILING(x,y) (((x) + (y) - 1) / (y))

// A cube is 4x4x4 values, with last positions overlapping with next one ("owning" 3x3x3 values)
#define CUBE_COUNT (CEILING(CHUNK_SIZE, 3))
#define END_CUBE_OFFSET ((3 - CHUNK_SIZE % 3) % 3)

class FVoxelData;

struct FVoxelVertex
{
	FVector Position;
	FVector Normal;
	FColor Color;

	FVoxelVertex() = default;
	FVoxelVertex(const FVector& P, const FVector&N, const FColor& C) : Position(P), Normal(N), Color(C) {}
};

struct FVoxelIntermediateChunk
{
	int8 LOD;
	TArray<int32> IndexBuffer;
	TArray<FVoxelVertex> VertexBuffer;

	void Reset();

	void InitSectionBuffers(TArray<FVoxelProcMeshVertex>& OutVertexBuffer, TArray<int32>& OutIndexBuffer, uint8 TransitionsMask) const;

	static FVector GetTranslated(const FVector& Vertex, const FVector& Normal, uint8 TransitionsMask, uint8 LOD);
};

class FVoxelPolygonizer
{
public:
	FVoxelPolygonizer(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition, bool bCreateAdditionalVerticesForMaterialsTransitions, bool bEnableNormals);

	bool CreateChunk(FVoxelIntermediateChunk& OutChunk);
	
	// For NormalImpl
	float GetValue(int X, int Y, int Z) const;

private:
	const int LOD;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const bool bCreateAdditionalVerticesForMaterialsTransitions;
	const bool bEnableNormals;

	// Cache of the sign of the values. Can lead to crash if value changed between cache and 2nd access
	uint64 CachedSigns[CUBE_COUNT * CUBE_COUNT * CUBE_COUNT];

	// +1: for end edge, +2: for normals
	float CachedValues[(CHUNK_SIZE + 3) * (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3)];
	FVoxelMaterial CachedMaterials[(CHUNK_SIZE + 3) * (CHUNK_SIZE + 3) * (CHUNK_SIZE + 3)];

	// Cache to get index of already created vertices
	int Cache[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE][3];

	// For vertices that are EXACTLY on the grid
	int IntegerCoordinates[CHUNK_SIZE + 1][CHUNK_SIZE + 1][CHUNK_SIZE + 1];

	FORCEINLINE int Size() const;
	// Step between cubes
	FORCEINLINE int Step() const;

	void GetNormal(const FVector& Position, FVector& Result) const;
	void GetValueAndMaterialNoCache(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const;
	void GetValueAndMaterialFromCache(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial) const;

	FORCEINLINE void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex);

	bool InterpolateX(int MinX, int MaxX, int Y,    int Z,    FVector& OutVector, uint8& OutAlpha);
	bool InterpolateY(int X,    int MinY, int MaxY, int Z,    FVector& OutVector, uint8& OutAlpha);
	bool InterpolateZ(int X,    int Y,    int MinZ, int MaxZ, FVector& OutVector, uint8& OutAlpha);
};

class FVoxelPolygonizerForTransitions
{
public:
	FVoxelPolygonizerForTransitions(int LOD, FVoxelData* Data, const FIntVector& ChunkPosition);

	bool CreateTransitions(TArray<FVoxelProcMeshVertex>& OutVertexBuffer, TArray<int32>& OutIndexBuffer, uint8 TransitionsMask);

private:
	const int LOD;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	
	int Cache2D[6][CHUNK_SIZE + 1][CHUNK_SIZE + 1][10];

	FORCEINLINE int Size() const;
	FORCEINLINE int Step() const;
	
	FORCEINLINE void SaveVertex2D(int Direction, int X, int Y, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex2D(int Direction, int X, int Y, short CacheDirection, short EdgeIndex) const;

	void GetNormal(const FVector& Position, FVector& Result) const;
	void GetValueAndMaterial(EVoxelDirection Direction, int X, int Y, float& OutValue, FVoxelMaterial& OutMaterial) const;

	FORCEINLINE void GlobalToLocal2D(int Size, EVoxelDirection Direction, int GX, int GY, int GZ, int& OutLX, int& OutLY, int& OutLZ) const;
	FORCEINLINE void Local2DToGlobal(int Size, EVoxelDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ) const;

	bool InterpolateX(EVoxelDirection Direction, int MinX, int MaxX, int Y,    FVector& OutVector, uint8& OutAlpha) const;
	bool InterpolateY(EVoxelDirection Direction, int X,    int MinY, int MaxY, FVector& OutVector, uint8& OutAlpha) const;
};