// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelEdGraph.h"
#include "VoxelGraphGenerator.h"

UVoxelGraphGenerator* UVoxelEdGraph::GetGenerator() const
{
	return CastChecked<UVoxelGraphGenerator>(GetOuter());
}