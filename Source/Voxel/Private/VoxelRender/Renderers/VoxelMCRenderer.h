// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDefaultRenderer.h"
#include "VoxelRenderChunk.h"

class FVoxelMCRenderChunk : public FVoxelRenderChunk
{
public:
	using FVoxelRenderChunk::FVoxelRenderChunk;

	TUniquePtr<FVoxelPolygonizerAsyncWork> GetNewTask() override;
	TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> GetNewTransitionTask() override;
};

class FVoxelMCRenderer : public FVoxelDefaultRenderer
{
public:
	using FVoxelDefaultRenderer::FVoxelDefaultRenderer;

	virtual TSharedRef<FVoxelRenderChunk, ESPMode::ThreadSafe> GetRenderChunk(uint8 LOD, const FIntBox& Bounds, const FVoxelRenderChunkSettings& Settings) override;
};