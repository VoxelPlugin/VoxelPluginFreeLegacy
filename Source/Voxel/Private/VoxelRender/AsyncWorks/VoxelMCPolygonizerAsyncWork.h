// Copyright 2019 Phyronnaz

#pragma once

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

class FVoxelMCPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	FVoxelMCPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
		: FVoxelPolygonizerAsyncWork(Chunk, "FVoxelMCPolygonizerAsyncWork")
	{
	}
	
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};

class FVoxelMCTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	FVoxelMCTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
		: FVoxelTransitionsPolygonizerAsyncWork(Chunk, "FVoxelMCTransitionsPolygonizerAsyncWork")
	{
	}
	
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};