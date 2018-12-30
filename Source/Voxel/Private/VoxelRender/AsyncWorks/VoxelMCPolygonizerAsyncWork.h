// Copyright 2018 Phyronnaz

#pragma once

#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRender/VoxelIntermediateChunk.h"

class FVoxelMCPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	using FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork;
	
	virtual void DoWork() override;

};

class FVoxelMCTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	using FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork;
	
	virtual void DoWork() override;
};
