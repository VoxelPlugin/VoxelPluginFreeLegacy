// Copyright 2018 Phyronnaz

#include "VoxelCubicRenderer.h"
#include "VoxelWorld.h"
#include "VoxelRender/AsyncWorks/VoxelCubicPolygonizerAsyncWork.h"

TUniquePtr<FVoxelPolygonizerAsyncWork> FVoxelCubicRenderChunk::GetNewTask(const FVoxelPreviousGrassInfo& InPreviousGrassInfo) const
{
	return MakeUnique<FVoxelCubicPolygonizerAsyncWork>(
		LOD,
		Render->GetSquaredDistanceToInvokers(Position),
		Position,
		Render->World,
		Render->World->GetDataSharedPtr(),
		false,
		false,
		InPreviousGrassInfo,
		(FVoxelRenderChunk*)this
		);
}

TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> FVoxelCubicRenderChunk::GetNewTransitionTask(uint8 InTransitionsMask) const
{
	return MakeUnique<FVoxelCubicTransitionsPolygonizerAsyncWork>(
		LOD,
		Render->GetSquaredDistanceToInvokers(Position),
		Position,
		Render->World,
		Render->World->GetDataSharedPtr(),
		(FVoxelRenderChunk*)this, 
		InTransitionsMask);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<FVoxelRenderChunk> FVoxelCubicRenderer::GetRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds)
{
	return MakeShared<FVoxelCubicRenderChunk>(Render, LOD, Bounds);
}
