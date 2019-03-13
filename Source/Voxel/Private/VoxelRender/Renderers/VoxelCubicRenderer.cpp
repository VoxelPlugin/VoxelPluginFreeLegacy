// Copyright 2019 Phyronnaz

#include "VoxelCubicRenderer.h"
#include "VoxelRender/AsyncWorks/VoxelCubicPolygonizerAsyncWork.h"

TUniquePtr<FVoxelPolygonizerAsyncWork> FVoxelCubicRenderChunk::GetNewTask()
{
	return MakeUnique<FVoxelCubicPolygonizerAsyncWork>(this);
}

TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> FVoxelCubicRenderChunk::GetNewTransitionTask()
{
	return MakeUnique<FVoxelCubicTransitionsPolygonizerAsyncWork>(this);
}

TSharedRef<FVoxelRenderChunk, ESPMode::ThreadSafe> FVoxelCubicRenderer::GetRenderChunk(uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& InSettings)
{
	return MakeShared<FVoxelCubicRenderChunk, ESPMode::ThreadSafe>(this, LOD, Bounds, InSettings);
}
