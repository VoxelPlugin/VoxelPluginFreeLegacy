#pragma once

#include "CoreMinimal.h"
#include "VoxelProceduralMeshComponent.h"
#include "TransitionDirection.h"

class FValueOctree;
class FVoxelData;
struct FVoxelMaterial;

class FVoxelPolygonizer
{
public:
	FVoxelPolygonizer(int Depth, FVoxelData* Data, FIntVector ChunkPosition, TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes, bool bComputeTransitions, bool bComputeCollisions, bool bEnableAmbientOcclusion, int RayMaxDistance, int RayCount);

	void CreateSection(FVoxelProcMeshSection& OutSection);

private:
	int const Depth;
	FVoxelData* const Data;
	FIntVector const ChunkPosition;
	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;

	const bool bComputeTransitions;
	const bool bComputeCollisions;
	const bool bEnableAmbientOcclusion;

	const int RayMaxDistance;
	const int RayCount;


	FValueOctree* LastOctree;

	// Cache of the sign of the values. Can lead to crash if value changed between cache and 2nd access
	uint64 CachedSigns[216];

	float CachedValues[18 * 18 * 18];
	FVoxelMaterial CachedMaterials[18 * 18 * 18];

	// Cache to get index of already created vertices
	int Cache[18][18][18][3];

	int Cache2D[6][17][17][7]; // Edgeindex: 0 -> 8; 1 -> 9; 2 -> Not used; 3-6 -> 3-6

	FORCEINLINE int Size();
	// Step between cubes
	FORCEINLINE int Step();

	// Too slow
	//FColor GetMajorColor(int X, int Y, int Z, uint32 CellWidth);

	FORCEINLINE void GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial);
	FORCEINLINE void Get2DValueAndMaterial(TransitionDirection Direction, int X, int Y, float& OutValue, FVoxelMaterial& OutMaterial);

	FORCEINLINE void SaveVertex(int X, int Y, int Z, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex(int X, int Y, int Z, short Direction, short EdgeIndex);

	FORCEINLINE void SaveVertex2D(TransitionDirection Direction, int X, int Y, short EdgeIndex, int Index);
	FORCEINLINE int LoadVertex2D(TransitionDirection Direction, int X, int Y, short CacheDirection, short EdgeIndex);

	void InterpolateX(int MinX, int MaxX, const int Y, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY(const int X, int MinY, int MaxY, const int Z, FVector& OutVector, uint8& OutAlpha);
	void InterpolateZ(const int X, const int Y, int MinZ, int MaxZ, FVector& OutVector, uint8& OutAlpha);

	void InterpolateX2D(TransitionDirection Direction, int MinX, int MaxX, const int Y, FVector& OutVector, uint8& OutAlpha);
	void InterpolateY2D(TransitionDirection Direction, const int X, int MinY, int MaxY, FVector& OutVector, uint8& OutAlpha);

	FORCEINLINE void GlobalToLocal2D(int Size, TransitionDirection Direction, int GX, int GY, int GZ, int& OutLX, int& OutLY, int& OutLZ);
	FORCEINLINE void Local2DToGlobal(int Size, TransitionDirection Direction, int LX, int LY, int LZ, int& OutGX, int& OutGY, int& OutGZ);

	FORCEINLINE FVector GetTranslated(const FVector Vertex, const FVector Normal);
};