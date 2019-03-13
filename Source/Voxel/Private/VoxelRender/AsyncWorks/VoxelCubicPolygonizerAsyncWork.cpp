// Copyright 2019 Phyronnaz

#include "VoxelCubicPolygonizerAsyncWork.h"
#include "VoxelRender/Polygonizers/VoxelCubicPolygonizer.h"

TSharedRef<FVoxelPolygonizerBase> FVoxelCubicPolygonizerAsyncWork::GetPolygonizer()
{
	return MakeShared<FVoxelCubicPolygonizer>(this);
}

EVoxelStatsType FVoxelCubicPolygonizerAsyncWork::GetTaskType()
{
	return EVoxelStatsType::NormalCubic;
}

TSharedRef<FVoxelPolygonizerBase> FVoxelCubicTransitionsPolygonizerAsyncWork::GetPolygonizer()
{
	return MakeShared<FVoxelCubicTransitionsPolygonizer>(this);
}

EVoxelStatsType FVoxelCubicTransitionsPolygonizerAsyncWork::GetTaskType()
{
	return EVoxelStatsType::TransitionsCubic;
}
