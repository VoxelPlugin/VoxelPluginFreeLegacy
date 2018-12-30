// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelLODRenderer.h"

class FVoxelCubicRenderChunk : public FVoxelRenderChunk
{
public:
	using FVoxelRenderChunk::FVoxelRenderChunk;

	TUniquePtr<FVoxelPolygonizerAsyncWork> GetNewTask(const FVoxelPreviousGrassInfo& InPreviousGrassInfo) const override;
	TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> GetNewTransitionTask(uint8 InTransitionsMask) const override;
};

class FVoxelCubicRenderer : public FVoxelLODRenderer
{
public:
	using FVoxelLODRenderer::FVoxelLODRenderer;

	virtual TSharedRef<FVoxelRenderChunk> GetRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds) override;
};