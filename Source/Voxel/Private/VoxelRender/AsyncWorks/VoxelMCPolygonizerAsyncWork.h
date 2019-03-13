// Copyright 2019 Phyronnaz

#pragma once

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

class FVoxelMCPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	using FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork;
	
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};

class FVoxelMCTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	using FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork;
	
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};