// Copyright 2019 Phyronnaz

#include "VoxelMCPolygonizerAsyncWork.h"
#include "VoxelRender/Polygonizers/VoxelMCPolygonizer.h"

TSharedRef<FVoxelPolygonizerBase> FVoxelMCPolygonizerAsyncWork::GetPolygonizer()
{
	return MakeShared<FVoxelMCPolygonizer>(this);
}

EVoxelStatsType FVoxelMCPolygonizerAsyncWork::GetTaskType()
{
	return EVoxelStatsType::NormalMarchingCubes;
}

TSharedRef<FVoxelPolygonizerBase> FVoxelMCTransitionsPolygonizerAsyncWork::GetPolygonizer()
{
	return MakeShared<FVoxelMCTransitionsPolygonizer>(this);
}

EVoxelStatsType FVoxelMCTransitionsPolygonizerAsyncWork::GetTaskType()
{
	return EVoxelStatsType::TransitionsMarchingCubes;
}
