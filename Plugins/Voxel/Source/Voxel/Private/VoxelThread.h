// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "VoxelRender.h"

class AVoxelChunk;

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

	FProcMeshSection Section;

	VoxelRender* Render;

	void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(VoxelThread, STATGROUP_ThreadPoolAsyncTasks);
	}
};
