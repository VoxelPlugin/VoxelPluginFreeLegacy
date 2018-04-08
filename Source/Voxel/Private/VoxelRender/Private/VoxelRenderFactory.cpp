// Copyright 2018 Phyronnaz

#include "VoxelRenderFactory.h"
#include "VoxelPrivate.h"
#include "VoxelWorld.h"
#include "LODVoxelRender.h"
#include "CubicVoxelRender.h"

TSharedPtr<IVoxelRender> FVoxelRenderFactory::GetVoxelRender(EVoxelRenderType VoxelRender, AVoxelWorld* World, AActor* ChunksOwner)
{
	switch (VoxelRender)
	{
	case EVoxelRenderType::WithLOD:
		return MakeShared<FLODVoxelRender>(World, ChunksOwner);
	case EVoxelRenderType::Cubic:
		return MakeShared<FCubicVoxelRender>(World, ChunksOwner);
	default:
		check(false);
		return TSharedPtr<IVoxelRender>();
	}
}
