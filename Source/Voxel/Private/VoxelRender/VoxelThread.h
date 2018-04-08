// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelProceduralMeshComponent.h"
#include "VoxelThreadPool.h"
#include "VoxelPolygonizer.h"

class AVoxelWorld;
class FVoxelPolygonizer;
class UVoxelChunkComponent;
class FVoxelData;
class FVoxelWorldGeneratorInstance;
class AVoxelActor;


class FVoxelAsyncWork : public IVoxelQueuedWork
{
public:
	FVoxelAsyncWork();
	~FVoxelAsyncWork() override;

	virtual void DoWork() = 0;

	virtual void DoThreadedWork() final;
	virtual void Abandon() final;

	bool IsDone() const;
	void WaitForCompletion();

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;
};

/**
 * Thread to create mesh
 */
class FAsyncPolygonizerWork : public FVoxelAsyncWork
{
public:
	const int LOD;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const FIntVector PositionOffset;

	const AVoxelWorld* const World;
	

	// Mesh Output
	FVoxelIntermediateChunk Chunk;

	FAsyncPolygonizerWork(
		int LOD,
		FVoxelData* Data,
		const FIntVector& ChunkPosition,
		const FIntVector& PositionOffset,
		AVoxelWorld* World
		);
	
	virtual void DoWork() override;
	virtual int GetPriority() const override;

private:
	FThreadSafeCounter IsDoneCounter;
	FEvent* DoneEvent;
	FCriticalSection DoneSection;

};

/**
 * Thread to create transitions
 */
class FAsyncPolygonizerForTransitionsWork : public FVoxelAsyncWork
{
public:
	const int LOD;
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const uint8 TransitionsMask;

	// Output
	TArray<FVoxelProcMeshVertex> VertexBuffer;
	TArray<int32> IndexBuffer;

	FAsyncPolygonizerForTransitionsWork(
		int LOD,
		FVoxelData* Data,
		const FIntVector& ChunkPosition,
		uint8 TransitionsMask);
	
	virtual void DoWork() override;
	virtual int GetPriority() const override;
};