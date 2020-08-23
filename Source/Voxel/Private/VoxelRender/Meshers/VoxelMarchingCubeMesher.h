// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelRender/Meshers/VoxelMesher.h"

#define CHUNK_SIZE_WITH_END_EDGE (RENDER_CHUNK_SIZE + 1)
#define CHUNK_SIZE_WITH_NORMALS (RENDER_CHUNK_SIZE + 3)

#define EDGE_INDEX_COUNT 4

class FVoxelMarchingCubeMesher : public FVoxelMesher
{
public:
	using FVoxelMesher::FVoxelMesher;

protected:
	virtual FVoxelIntBox GetBoundsToCheckIsEmptyOn() const override final;
	virtual FVoxelIntBox GetBoundsToLock() const override final;

	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) override final;
	virtual void CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices) override final;

public:	
	// For GetGradient template
	FORCEINLINE FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 InLOD) const
	{
		checkVoxelSlow(LOD == 0);
		checkVoxelSlow(InLOD == 0);
		checkVoxelSlow(-1 <= X && X < CHUNK_SIZE_WITH_NORMALS - 1);
		checkVoxelSlow(-1 <= Y && Y < CHUNK_SIZE_WITH_NORMALS - 1);
		checkVoxelSlow(-1 <= Z && Z < CHUNK_SIZE_WITH_NORMALS - 1);
		return CachedValues[(X + 1) + (Y + 1) * CHUNK_SIZE_WITH_NORMALS + (Z + 1) * CHUNK_SIZE_WITH_NORMALS * CHUNK_SIZE_WITH_NORMALS];
	}

private:
	// Use LOD0 size as it's bigger
	using FCachedValues = TVoxelStaticArray<FVoxelValue, CHUNK_SIZE_WITH_NORMALS * CHUNK_SIZE_WITH_NORMALS * CHUNK_SIZE_WITH_NORMALS>;
	using FCache = TVoxelStaticArray<int32, RENDER_CHUNK_SIZE * RENDER_CHUNK_SIZE * EDGE_INDEX_COUNT>;

	TUniquePtr<FCachedValues> CachedValuesStorage = MakeUnique<FCachedValues>();
	TUniquePtr<FCache> CacheStorageA = MakeUnique<FCache>();
	TUniquePtr<FCache> CacheStorageB = MakeUnique<FCache>();
	
	TUniquePtr<FVoxelConstDataAccelerator> Accelerator;

	FVoxelValue* RESTRICT const CachedValues = CachedValuesStorage->GetData();

	// Cache to get index of already created vertices
	int32* RESTRICT CurrentCache = CacheStorageA->GetData();
	int32* RESTRICT OldCache = CacheStorageB->GetData();

private:
	// T: will be created as T(IntersectionPoint, MaterialPosition)
	template<typename T>
	bool CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices);

private:
	static int32 GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY);
	
	friend class FMarchingCubeHelpers;
};

#define TRANSITION_EDGE_INDEX_COUNT 10

class FVoxelMarchingCubeTransitionsMesher : public FVoxelTransitionsMesher
{
public:
	using FVoxelTransitionsMesher::FVoxelTransitionsMesher;

protected:
	virtual FVoxelIntBox GetBoundsToCheckIsEmptyOn() const override final;
	virtual FVoxelIntBox GetBoundsToLock() const override final;
	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) override final;

private:
	TUniquePtr<FVoxelConstDataAccelerator> Accelerator;
	TVoxelStaticArray<int32, RENDER_CHUNK_SIZE * RENDER_CHUNK_SIZE * TRANSITION_EDGE_INDEX_COUNT> Cache2D;

private:
	// T: will be created as T(IntersectionPoint, MaterialPosition, bNeedToTranslate)
	template<typename T>
	bool CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices);
	template<uint8 Direction, typename T>
	bool CreateGeometryForDirection(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices);

private:
	static int32 GetCacheIndex(int32 EdgeIndex, int32 LX, int32 LY);
	template<uint8 Direction>
	FVoxelValue GetValue(int32 X, int32 Y, int32 InLOD) const;
	template<uint8 Direction>
	FIntVector Local2DToGlobal(int32 X, int32 Y, int32 Z) const;

	friend class FMarchingCubeHelpers;
};
