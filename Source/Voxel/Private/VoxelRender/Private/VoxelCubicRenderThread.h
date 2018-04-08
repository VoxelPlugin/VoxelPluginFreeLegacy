// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelThread.h"
#include "VoxelProceduralMeshComponent.h"

class AVoxelWorld;
class FVoxelPolygonizer;
class UVoxelChunkComponent;
class FVoxelData;
class FVoxelWorldGeneratorInstance;
class AVoxelActor;

/**
 * Thread to create mesh
 */
class FAsyncCubicPolygonizerWork : public FVoxelAsyncWork
{
public:
	FVoxelData* const Data;
	const FIntVector ChunkPosition;
	const FIntVector PositionOffset;

	const AVoxelWorld* const World;
	

	// Mesh Output
	FVoxelProcMeshSection Section;

	FAsyncCubicPolygonizerWork(
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