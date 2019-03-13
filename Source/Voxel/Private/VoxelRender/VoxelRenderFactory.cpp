// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelRenderFactory.h"
#include "VoxelWorld.h"
#include "Renderers/VoxelMCRenderer.h"
#include "Renderers/VoxelCubicRenderer.h"
#include "AsyncWorks/VoxelMCPolygonizerAsyncWork.h"
#include "AsyncWorks/VoxelCubicPolygonizerAsyncWork.h"
#include "VoxelData/VoxelData.h"

TSharedRef<IVoxelRenderer> FVoxelRenderFactory::GetVoxelRenderer(EVoxelRenderType RenderType, const FVoxelRendererSettings& Settings)
{
	switch (RenderType)
	{
	case EVoxelRenderType::MarchingCubes:
		return MakeShared<FVoxelMCRenderer>(Settings);
	case EVoxelRenderType::Cubic:
		return MakeShared<FVoxelCubicRenderer>(Settings);
	default:
		check(false);
		return TSharedPtr<IVoxelRenderer>().ToSharedRef();
	}
}
