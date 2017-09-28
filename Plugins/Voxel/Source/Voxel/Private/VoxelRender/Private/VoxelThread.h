// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "VoxelRender.h"

class AVoxelChunk;

/**
 * Thread to create mesh section
 */
class VoxelThread : public IQueuedWork
{
public:
	VoxelThread(AVoxelChunk* VoxelChunk);
	virtual ~VoxelThread() override;

	virtual void DoThreadedWork() override;

	// Do not call
	virtual void Abandon() override;

	bool IsDone();

	FProcMeshSection& GetSection();

private:
	FThreadSafeCounter IsDoneCounter;
	FProcMeshSection Section;
	VoxelPolygonizer* Render;
};
