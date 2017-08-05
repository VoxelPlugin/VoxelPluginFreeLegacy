// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"

class AVoxelChunk;
struct VoxelChunkStruct;

/**
 *
 */
class VoxelThread : public FNonAbandonableTask
{
	friend class FAsyncTask<VoxelThread>;

public:
	VoxelThread(AVoxelChunk* voxelChunk);
	~VoxelThread();

	AVoxelChunk* VoxelChunk;
	VoxelChunkStruct* VoxelStruct;

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(VoxelThread, STATGROUP_ThreadPoolAsyncTasks);
	}
};
