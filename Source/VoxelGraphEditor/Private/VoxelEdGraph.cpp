// Copyright 2020 Phyronnaz

#include "VoxelEdGraph.h"
#include "VoxelGraphGenerator.h"

UVoxelGraphGenerator* UVoxelEdGraph::GetGenerator() const
{
	return CastChecked<UVoxelGraphGenerator>(GetOuter());
}