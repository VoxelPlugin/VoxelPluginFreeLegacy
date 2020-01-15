// Copyright 2020 Phyronnaz

#include "VoxelEdGraph.h"
#include "VoxelGraphGenerator.h"

UVoxelGraphGenerator* UVoxelEdGraph::GetWorldGenerator() const
{
	return CastChecked<UVoxelGraphGenerator>(GetOuter());
}