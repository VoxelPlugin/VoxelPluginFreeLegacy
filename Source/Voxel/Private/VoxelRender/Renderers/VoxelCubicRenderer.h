// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefaultRenderer.h"
#include "VoxelRenderChunk.h"

class FVoxelCubicRenderChunk : public FVoxelRenderChunk
{
public:
	using FVoxelRenderChunk::FVoxelRenderChunk;

	TUniquePtr<FVoxelPolygonizerAsyncWork> GetNewTask() override;
	TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> GetNewTransitionTask() override;
};

class FVoxelCubicRenderer : public FVoxelDefaultRenderer
{
public:
	using FVoxelDefaultRenderer::FVoxelDefaultRenderer;

	virtual TSharedRef<FVoxelRenderChunk, ESPMode::ThreadSafe> GetRenderChunk(uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& Settings) override;
};