// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelPolygonizerAsyncWork.h"
#include "VoxelRender/VoxelIntermediateChunk.h"

class FVoxelCubicPolygonizerAsyncWork : public FVoxelPolygonizerAsyncWork
{
public:
	using FVoxelPolygonizerAsyncWork::FVoxelPolygonizerAsyncWork;
	
	virtual void DoWork() override;
};

class FVoxelCubicTransitionsPolygonizerAsyncWork : public FVoxelTransitionsPolygonizerAsyncWork
{
public:	
	using FVoxelTransitionsPolygonizerAsyncWork::FVoxelTransitionsPolygonizerAsyncWork;
	
	virtual void DoWork() override;
};
