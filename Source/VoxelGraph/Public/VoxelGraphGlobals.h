// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

#define ENABLE_VOXELGRAPH_CHECKS 0

#define MAX_VOXELNODE_PINS 256
#define MAX_VOXELFUNCTION_ARGS 256
#define MAX_VOXELGRAPH_OUTPUTS 256

#if ENABLE_VOXELGRAPH_CHECKS
#define checkVoxelGraph(...) check(__VA_ARGS__)
#else
#define checkVoxelGraph(...)
#endif

#define DEPRECATED_VOXEL_GRAPH_FUNCTION() UE_DEPRECATED(0, "Outdated C++ voxel graph, you should compile it to C++ again")

#define VOXEL_GRAPH_GENERATED_VERSION 1