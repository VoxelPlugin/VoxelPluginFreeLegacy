// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelUserDefinitions.h"

/**
 * To change a definition, add it to VoxelUserDefinitions.h
 */

// Enable this to enable double precision in world generators and voxel graphs
#ifndef VOXEL_DOUBLE_PRECISION
#define VOXEL_DOUBLE_PRECISION 0
#endif

// Enables slow voxel checks
#ifndef VOXEL_DEBUG
#define VOXEL_DEBUG 0
#endif

// Disable if the stats file is too big
// Expensive
#ifndef VOXEL_SLOW_STATS
#define VOXEL_SLOW_STATS 0
#endif

// Will check that the data octree is locked for read/write
// Expensive
#ifndef DO_THREADSAFE_CHECKS
#define DO_THREADSAFE_CHECKS VOXEL_DEBUG
#endif

// Size of a render chunk
// Bigger = less draw calls
// Smaller = faster edits
// Must be a power of 2
#ifndef RENDER_CHUNK_SIZE
#define RENDER_CHUNK_SIZE 32
#endif

// Size of a data chunk
// Should leave it to default
#ifndef DATA_CHUNK_SIZE
#define DATA_CHUNK_SIZE 16
#endif

// No tessellation support on some platforms
#ifndef ENABLE_TESSELLATION
#define ENABLE_TESSELLATION (!PLATFORM_ANDROID && !PLATFORM_SWITCH)
#endif


// Enables recording detailed mesher stats (eg profiles every GetValue call)
// In my tests, adds a cost < 5% of the total generation time with a flat world generator,
// which is the worst cast for this as no time is spent generating values with it.
// Should be cheap enough to leave on
// NOTE: stats are recorded all the time and can only ever be cleared by the user! They are relatively small so should have no impact on memory,
// but might be an issue at some point!
#ifndef ENABLE_MESHER_STATS
#define ENABLE_MESHER_STATS (!UE_BUILD_SHIPPING)
#endif

// Records memory stats about voxels in addition to UE's stat system
// Unlike UE's stat system, it can be used in shipping builds
// Use UVoxelBlueprintLibrary::GetMemoryUsageInMB to get the info
#ifndef ENABLE_VOXEL_MEMORY_STATS
#define ENABLE_VOXEL_MEMORY_STATS 1
#endif

// Record stats about voxel data accelerators
// Minimal impact on performance
#ifndef VOXEL_DATA_ACCELERATOR_STATS
#define VOXEL_DATA_ACCELERATOR_STATS VOXEL_DEBUG
#endif

// No support for indices optimizations on some platforms
// Note: I have yet to find any performance improvements due to this
#ifndef ENABLE_OPTIMIZE_INDICES
#define ENABLE_OPTIMIZE_INDICES PLATFORM_WINDOWS
#endif

#ifndef EIGHT_BITS_VOXEL_VALUE
#define EIGHT_BITS_VOXEL_VALUE 0
#endif

// If true, Voxel Materials will default to R = G = B = A = 255
// else to R = G = B = A = 0
#ifndef VOXEL_MATERIAL_DEFAULT_IS_WHITE
#define VOXEL_MATERIAL_DEFAULT_IS_WHITE 0
#endif

/**
 * Voxel material config: use those to reduce the size of a FVoxelMaterial
 */

#ifndef VOXEL_MATERIAL_ENABLE_R
#define VOXEL_MATERIAL_ENABLE_R 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_G
#define VOXEL_MATERIAL_ENABLE_G 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_B
#define VOXEL_MATERIAL_ENABLE_B 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_A
#define VOXEL_MATERIAL_ENABLE_A 1
#endif

// Each additional UV channel uses 2 bytes
#ifndef VOXEL_MATERIAL_ENABLE_UV0
#define VOXEL_MATERIAL_ENABLE_UV0 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV1
#define VOXEL_MATERIAL_ENABLE_UV1 1
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV2
#define VOXEL_MATERIAL_ENABLE_UV2 0
#endif
#ifndef VOXEL_MATERIAL_ENABLE_UV3
#define VOXEL_MATERIAL_ENABLE_UV3 0
#endif