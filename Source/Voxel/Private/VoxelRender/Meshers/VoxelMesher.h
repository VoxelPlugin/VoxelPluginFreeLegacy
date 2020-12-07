// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelStatHelpers.h"

struct FVoxelChunkMesh;
class FVoxelData;
class IVoxelRenderer;
class FVoxelDataLockInfo;
class FVoxelRuntimeSettings;
class FVoxelRuntimeDynamicSettings;

#if ENABLE_MESHER_STATS
#define MESHER_TIME_SCOPE(Time) VOXEL_SCOPED_STAT(Times.Time, 1)
#define MESHER_TIME_INLINE(Time, Expr) VOXEL_INLINE_STAT(Times.Time, 1, Expr)

#define MESHER_TIME_SCOPE_VALUES(Count) VOXEL_SCOPED_STAT(Times.Values, Count)
#define MESHER_TIME_INLINE_VALUES(Count, Expr) VOXEL_INLINE_STAT(Times.Values, Count, Expr)

#define MESHER_TIME_SCOPE_MATERIALS(Count) VOXEL_SCOPED_STAT(Times.Materials, Count)
#define MESHER_TIME_INLINE_MATERIALS(Count, Expr) VOXEL_INLINE_STAT(Times.Materials, Count, Expr)
#else
#define MESHER_TIME_SCOPE(Time)
#define MESHER_TIME_INLINE(Time, Expr) Expr

#define MESHER_TIME_SCOPE_VALUES(Count)
#define MESHER_TIME_INLINE_VALUES(Count, Expr) Expr

#define MESHER_TIME_SCOPE_MATERIALS(Count)
#define MESHER_TIME_INLINE_MATERIALS(Count, Expr) Expr
#endif

struct FVoxelMesherTimes
{
	FVoxelStatEntry Values;
	FVoxelStatEntry Materials;

	FVoxelStatEntry Normals;
	FVoxelStatEntry UVs;
	FVoxelStatEntry CreateChunk;

	FVoxelStatEntry FindFaces;
	FVoxelStatEntry AddFaces;
	FVoxelStatEntry GreedyMeshing;
	FVoxelStatEntry CollisionCubes;
	
	FVoxelStatEntry FinishCreatingChunk;
	FVoxelStatEntry DistanceField;
};

class FVoxelMesherBase
{
public:
	const int32 LOD;
	const int32 Step;
	const int32 Size;
	const FIntVector ChunkPosition;
	const FVoxelRuntimeSettings& Settings;
	const FVoxelRuntimeDynamicSettings& DynamicSettings;
	const FVoxelData& Data;
	const IVoxelRenderer& Renderer;
	const bool bIsTransitions;

	FVoxelMesherBase(
		int32 LOD,
		const FIntVector& ChunkPosition,
		const IVoxelRenderer& Renderer,
		const FVoxelData& Data,
		bool bIsTransitions);
	virtual ~FVoxelMesherBase();

	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunk() = 0;
	virtual void CreateGeometry(TArray<uint32>& Indices, TArray<FVector>& Vertices) = 0;
	
	TVoxelSharedPtr<FVoxelChunkMesh> CreateEmptyChunk() const;
	void FinishCreatingChunk(FVoxelChunkMesh& Chunk) const;

protected:
	virtual FVoxelIntBox GetBoundsToCheckIsEmptyOn() const = 0;
	virtual FVoxelIntBox GetBoundsToLock() const = 0;

	void UnlockData();
	
private:
	TUniquePtr<FVoxelDataLockInfo> LockInfo;

	void LockData();
	bool IsEmpty() const;

	friend class FVoxelMesher;
	friend class FVoxelTransitionsMesher;
};

class FVoxelMesher : public FVoxelMesherBase
{
public:
	FVoxelMesher(
		int32 LOD,
		const FIntVector& ChunkPosition,
		const IVoxelRenderer& Renderer,
		const FVoxelData& Data);

	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunk() override final;
	virtual void CreateGeometry(TArray<uint32>& Indices, TArray<FVector>& Vertices) override final;

protected:
	// Need to call UnlockData
	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) = 0;
	// Need to call UnlockData
	virtual void CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices) = 0;
};

class FVoxelTransitionsMesher : public FVoxelMesherBase
{
public:
	const uint8 TransitionsMask;
	const int32 HalfLOD;
	const int32 HalfStep;

	FVoxelTransitionsMesher(
		int32 LOD,
		const FIntVector& ChunkPosition,
		const IVoxelRenderer& Renderer,
		const FVoxelData& Data,
		uint8 TransitionsMask);

	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunk() override final;
	virtual void CreateGeometry(TArray<uint32>& Indices, TArray<FVector>& Vertices) override final;
	
protected:
	// Need to call UnlockData
	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) = 0;
};