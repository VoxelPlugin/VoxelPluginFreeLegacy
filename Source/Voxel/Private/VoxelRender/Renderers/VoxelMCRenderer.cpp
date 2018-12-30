// Copyright 2018 Phyronnaz

#include "VoxelMCRenderer.h"
#include "VoxelWorld.h"
#include "VoxelRender/AsyncWorks/VoxelMCPolygonizerAsyncWork.h"

TUniquePtr<FVoxelPolygonizerAsyncWork> FVoxelMCRenderChunk::GetNewTask(const FVoxelPreviousGrassInfo& InPreviousGrassInfo) const
{
	return MakeUnique<FVoxelMCPolygonizerAsyncWork>(
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

TUniquePtr<FVoxelTransitionsPolygonizerAsyncWork> FVoxelMCRenderChunk::GetNewTransitionTask(uint8 InTransitionsMask) const
{
	return  MakeUnique<FVoxelMCTransitionsPolygonizerAsyncWork>(
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

TSharedRef<FVoxelRenderChunk> FVoxelMCRenderer::GetRenderChunk(FVoxelLODRenderer* Render, uint8 LOD, const FIntBox& Bounds)
{
	return MakeShared<FVoxelMCRenderChunk>(Render, LOD, Bounds);
}
