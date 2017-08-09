// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

class AVoxelChunk;
struct VoxelChunkStruct;

/**
 * Thread to create mesh section
 */
class VoxelThread : public FNonAbandonableTask
{
	friend class FAsyncTask<VoxelThread>;

public:
	VoxelThread(AVoxelChunk* VoxelChunk);
	~VoxelThread();

	AVoxelChunk* VoxelChunk;
	VoxelChunkStruct* VoxelStruct;

	FProcMeshSection Section;

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(VoxelThread, STATGROUP_ThreadPoolAsyncTasks);
	}
};
