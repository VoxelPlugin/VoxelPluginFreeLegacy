#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "TransitionDirection.h"

class ValueOctree;
class VoxelData;
struct FVoxelMaterial;

class VoxelPolygonizer
{
public:
	VoxelPolygonizer(int Depth, VoxelData* Data, FIntVector ChunkPosition, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, bool bComputeTransitions);

	void CreateSection(FProcMeshSection& OutSection);

private:
	int const Depth;
	VoxelData* const Data;
	FIntVector const ChunkPosition;
	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;
	bool const bComputeTransitions;


	ValueOctree* LastOctree;

	uint64 CachedSigns[216];

	float CachedValues[18 * 18 * 18];
	FVoxelMaterial CachedMaterials[18 * 18 * 18];

	// Cache to get index of already created vertices
	int Cache[17][17][17][3];

	int Cache2D[6][17][17][7]; // Edgeindex: 0 -> 8; 1 -> 9; 2 -> Not used; 3-6 -> 3-6

	FORCEINLINE int Size();
	// Step between cubes
	FORCEINLINE int Step();

	// Too slow
	//FColor GetMajorColor(int X, int Y, int Z, uint32 CellWidth);

	void GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial);
	void Get2DValueAndMaterial(TransitionDirection Direction, int X, int Y, float& OutValue, FVoxelMaterial& OutMaterial);

	FORCEINLINE void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex);

	void SaveVertex2D(TransitionDirection Direction, int X, int Y, short EdgeIndex, int Index);
	int LoadVertex2D(TransitionDirection Direction, int X, int Y, short CacheDirection, short EdgeIndex);

	void InterpolateX(const int MinX, const int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY(const int X, const int MinY, const int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateZ(const int X, const int Y, const int MinZ, const int MaxZ, FVector& OutVector, uint8& OutAlpha);

	void InterpolateX2D(TransitionDirection Direction, const int MinX, const int MaxX, const int Y, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY2D(TransitionDirection Direction, const int X, const int MinY, const int MaxY, FVector& OutVector, uint8& OutAlpha);

	FORCEINLINE void GlobalToLocal2D(int Size, TransitionDirection Direction, int GX, int GY, int GZ, int& OutLX, int& OutLY, int& OutLZ);
	FORCEINLINE void Local2DToGlobal(int Size, TransitionDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ);

	FORCEINLINE FVector GetTranslated(const FVector Vertex, const FVector Normal);
};