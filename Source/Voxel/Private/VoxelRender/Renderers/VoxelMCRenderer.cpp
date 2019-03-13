// Copyright 2019 Phyronnaz

#include "VoxelMCRenderer.h"
#include "VoxelRender/AsyncWorks/VoxelMCPolygonizerAsyncWork.h"

TUniquePtr<FVoxelPolygonizerAsyncWork> FVoxelMCRenderChunk::GetNewTask()
{
	return MakeUnique<FVoxelMCPolygonizerAsyncWork>(this);
}

TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> FVoxelMCRenderChunk::GetNewTransitionTask()
{
	return  MakeUnique<FVoxelMCTransitionsPolygonizerAsyncWork>(this);
}

TSharedRef<FVoxelRenderChunk, ESPMode::ThreadSafe> FVoxelMCRenderer::GetRenderChunk(uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& InSettings)
{
	return MakeShared<FVoxelMCRenderChunk, ESPMode::ThreadSafe>(this, LOD, Bounds, InSettings);
}
