#pragma once

#include "CoreMinimal.h"
#include "VoxelProceduralMeshComponent.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"

class FVoxelData;
struct FVoxelMaterial;

class FVoxelCubicPolygonizer
{
public:
	FVoxelCubicPolygonizer(FVoxelData* Data, const FIntVector& ChunkPosition);

	bool CreateSection(FVoxelProcMeshSection& OutSection);

private:
	FVoxelData* const Data;
	FIntVector const ChunkPosition;

	float CachedValues[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];
	FVoxelMaterial CachedMaterials[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2)];

	FORCEINLINE float GetValue(int X, int Y, int Z) const;
	FORCEINLINE FVoxelMaterial GetMaterial(int X, int Y, int Z) const;
	FORCEINLINE bool IsInBounds(int X, int Y, int Z) const;
	FORCEINLINE void AddFace(int X, int Y, int Z, EVoxelDirection Direction, FVoxelProcMeshSection& Section);
};