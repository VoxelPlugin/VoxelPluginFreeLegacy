// Copyright 2019 Phyronnaz

#pragma once

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"

class FVoxelCubicPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	using FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork;
		
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};

class FVoxelCubicTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	using FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork;
		
protected:
	TSharedRef<FVoxelPolygonizerBase> GetPolygonizer() final;
	EVoxelStatsType GetTaskType() final;
};