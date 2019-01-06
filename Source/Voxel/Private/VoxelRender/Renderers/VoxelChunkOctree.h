// Copyright 2019 Phyronnaz

#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "VoxelOctree.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelDirection.h"
#include "HAL/ThreadSafeBool.h"
#include "Async/AsyncWork.h"

class FVoxelChunkOctree;

struct FVoxelChunkOctreeSettings
{
	const uint8 LODLimit;
	const uint8 LODLowerLimit;
	const int MaxOctreeLeaves;
	const FIntBox WorldBounds;
	const TArray<FIntVector> CameraPositions;
	// If Distance < SquaredDistances[LOD], subdivide
	const TArray<uint64> SquaredDistances;

	int NumberOfLeaves = 0;

	inline bool ShouldContinue() const { return NumberOfLeaves < MaxOctreeLeaves; }

	FVoxelChunkOctreeSettings(
		uint8 LODLimit,
		uint8 LODLowerLimit,
		int MaxOctreeLeaves,
		const FIntBox& WorldBounds,
		const TArray<FIntVector>& CameraPositions,
		const TArray<uint64>& SquaredDistances)
		: LODLimit(LODLimit)
		, LODLowerLimit(LODLowerLimit)
		, MaxOctreeLeaves(MaxOctreeLeaves)
		, WorldBounds(WorldBounds)
		, CameraPositions(CameraPositions)
		, SquaredDistances(SquaredDistances)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

class FAsyncOctreeBuilderTask : public IQueuedWork
{
public:
	TSet<FIntBox> ChunksToDelete;
	TSet<FIntBox> ChunksToCreate;
	TSharedPtr<FVoxelChunkOctree> NewOctree;
	TSharedPtr<FVoxelChunkOctree> OldOctree;
	TMap<FIntBox, uint8> TransitionsMasks;

	FAsyncOctreeBuilderTask(uint8 LOD, const FIntBox& WorldBounds);

public:
	void Init(
		const TArray<FIntVector>& CameraPositions,
		TSharedPtr<FVoxelChunkOctree> Octree,
		const TMap<uint8, float>& LODToMinDistance,
		float VoxelSize,
		uint8 LODLimit,
		uint8 LODLowerLimit,
		int MaxOctreeLeaves);
	void Reset();
	inline bool IsActive() const { return bIsActive; }
	inline bool WasCanceled() const { return bWasCanceled; }
	void Log() const;

private:
	void DoWork();

public:
	//~ Begin IQueuedWork Interface
	void DoThreadedWork() override;
	void Abandon() override {}
	//~ End IQueuedWork Interface

	bool IsDone() const { return bIsDone; }
	void Autodelete();

private:
	FCriticalSection DoneSection;
	FThreadSafeBool bIsDone = false;
	FThreadSafeBool bAutodelete = false;

private:
	const uint8 LOD;
	const FIntBox WorldBounds;

	TArray<FIntVector> CameraPositions;
	TSharedPtr<FVoxelChunkOctree> Octree;
	TMap<uint8, float> LODToMinDistance;
	float VoxelSize = 0;
	uint8 LODLimit = 0;
	uint8 LODLowerLimit = 0;
	int MaxOctreeLeaves = 0;

	bool bIsActive = false;

	bool bWasCanceled = false;
	bool bGameThreadUsed = false;
	std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> WorkStartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> WorkEndTime;
	int NumberOfLeaves = 0;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Create the octree for rendering and spawning VoxelChunks
 */
class FVoxelChunkOctree : public TVoxelOctree<FVoxelChunkOctree, CHUNK_SIZE>
{
public:
	FVoxelChunkOctreeSettings& Settings;
	FVoxelChunkOctree* const Root;

	FVoxelChunkOctree(FVoxelChunkOctreeSettings& Settings, uint8 LOD);
	FVoxelChunkOctree(FVoxelChunkOctree* Parent, uint8 ChildIndex);

	bool ShouldSubdivide() const;

	void GetLeavesBounds(TSet<FIntBox>& Bounds) const;
	void GetLeavesTransitionsMasks(TMap<FIntBox, uint8>& TransitionsMasks) const;

	FVoxelChunkOctree* GetAdjacentChunk(EVoxelDirection Direction) const;

private:
	bool bContinueInit;

	void Init();
};