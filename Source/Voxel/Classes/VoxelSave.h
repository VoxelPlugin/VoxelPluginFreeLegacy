// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelGlobals.h"
#include "VoxelSave.generated.h"

/**
 *	Save of one chunk
 */
struct FVoxelChunkSave
{
	uint64 Id;
	TArray<float, TFixedAllocator<DATA_CHUNK_TOTAL_SIZE>> Values;
	TArray<FVoxelMaterial, TFixedAllocator<DATA_CHUNK_TOTAL_SIZE>> Materials;

	FVoxelChunkSave();
	FVoxelChunkSave(uint64 Id, FIntVector Position, float Values[DATA_CHUNK_TOTAL_SIZE], FVoxelMaterial Materials[DATA_CHUNK_TOTAL_SIZE]);
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FVoxelChunkSave& Save)
{
	Ar << Save.Id;
	Ar << Save.Values;
	Ar << Save.Materials;

	return Ar;
}

///////////////////////////////////////////////////////////////////////////////

/**
 *	Compressed save of the world
 */
USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelWorldSave
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int LOD;

	UPROPERTY()
	TArray<uint8> Data;

	FVoxelWorldSave();

	void Init(int NewLOD, const TArray<FVoxelChunkSave>& ChunksList);
	void GetChunksQueue(TArray<FVoxelChunkSave>& SaveQueue) const;
};
