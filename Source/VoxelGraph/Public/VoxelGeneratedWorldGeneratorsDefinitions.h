// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

#if PLATFORM_COMPILER_CLANG
#define PRAGMA_GENERATED_VOXEL_GRAPH_START \
	_Pragma("clang diagnostic push") \
	//_Pragma("clang diagnostic ignored \"-Wnull-dereference\"")

#define PRAGMA_GENERATED_VOXEL_GRAPH_END \
	_Pragma("clang diagnostic pop")
#else
#define PRAGMA_GENERATED_VOXEL_GRAPH_START \
	__pragma(warning(push)) \
	__pragma(warning(disable: 4101 4701))

#define PRAGMA_GENERATED_VOXEL_GRAPH_END \
	__pragma(warning(pop))
#endif