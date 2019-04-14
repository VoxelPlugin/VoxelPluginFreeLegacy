// Copyright 2019 Phyronnaz

#pragma once

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

class FVoxelCubicPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	FVoxelCubicPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
		: FVoxelPolygonizerAsyncWork(Chunk, "FVoxelCubicPolygonizerAsyncWork")
	{
	}
		
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};

class FVoxelCubicTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	FVoxelCubicTransitionsPolygonizerAsyncWork(FVoxelRenderChunk* Chunk)
		: FVoxelTransitionsPolygonizerAsyncWork(Chunk, "FVoxelCubicTransitionsPolygonizerAsyncWork")
	{
	}
		
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};