// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

// Enable this to enable double precision in generators and voxel graphs
#ifndef VOXEL_DOUBLE_PRECISION
#define VOXEL_DOUBLE_PRECISION 0
#endif

// Assert on non finite numbers. Used to track NaNs.
#define VOXEL_ENABLE_NAN_DIAGNOSTIC 1

// Enables slow voxel checks
#ifndef VOXEL_DEBUG
#define VOXEL_DEBUG 0
#endif

// True when compilation speed does not matter
#ifndef VOXEL_PLUGIN_PACKAGED
#define VOXEL_PLUGIN_PACKAGED 1
#endif

// Disable if the stats file is too big
// Expensive
#ifndef VOXEL_SLOW_STATS
#define VOXEL_SLOW_STATS 0
#endif

// Will take longer to compile, but will be faster at runtime
#ifndef VOXEL_ENABLE_SLOW_OPTIMIZATIONS
#define VOXEL_ENABLE_SLOW_OPTIMIZATIONS (UE_BUILD_SHIPPING || VOXEL_PLUGIN_PACKAGED)
#endif

// Will check that the data octree is locked for read/write
// Expensive
#ifndef DO_THREADSAFE_CHECKS
#define DO_THREADSAFE_CHECKS VOXEL_DEBUG
#endif

// Size of a chunk processed by the mesher
// Must be a power of 2
// Bigger = larger rebuilds when editing, but potentially faster meshing
#ifndef MESHER_CHUNK_SIZE
#define MESHER_CHUNK_SIZE 32
#endif

// Base foliage chunk size, for LOD = 0
#ifndef FOLIAGE_CHUNK_SIZE
#define FOLIAGE_CHUNK_SIZE 32
#endif

// Size of a data chunk
// Should leave it to default
#ifndef DATA_CHUNK_SIZE
#define DATA_CHUNK_SIZE 16
#endif

// No tessellation support on some platforms
#ifndef ENABLE_TESSELLATION
#define ENABLE_TESSELLATION (!PLATFORM_ANDROID && !PLATFORM_SWITCH && ENGINE_MAJOR_VERSION < 5)
#endif

// Make UVoxelProceduralMeshComponent inherit from UModelComponent instead of UPrimitiveComponent
// to make unreal foliage painting in editor work.
// Huge hack, should disable if you don't use unreal in-editor foliage painting
#ifndef VOXEL_ENABLE_FOLIAGE_PAINT_HACK
#define VOXEL_ENABLE_FOLIAGE_PAINT_HACK 1
#endif

// Enables recording detailed mesher stats (eg profiles every GetValue call)
// In my tests, adds a cost < 5% of the total generation time with a flat generator,
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

// Use 8 bits voxel value
// Lower quality (mainly visible when using smooth), but 2x smaller
#ifndef EIGHT_BITS_VOXEL_VALUE
#define EIGHT_BITS_VOXEL_VALUE 0
#endif

// Only store one bit per voxel - mainly useful for cubic
#ifndef ONE_BIT_VOXEL_VALUE
#define ONE_BIT_VOXEL_VALUE 0
#endif

// If true, Voxel Materials will default to R = G = B = A = 255
// else to R = G = B = A = 0
#ifndef VOXEL_MATERIAL_DEFAULT_IS_WHITE
#define VOXEL_MATERIAL_DEFAULT_IS_WHITE 0
#endif

/**
 * Voxel material config: use those to reduce the size of a FVoxelMaterial
 * Alpha is used to store the single index in that mode
 */

// Quick way to disable all channels but the one used to store the single index
// Useful for magica voxel if you use a palette
#ifndef VOXEL_MATERIAL_SINGLE_INDEX_ONLY
#define VOXEL_MATERIAL_SINGLE_INDEX_ONLY 0
#endif

#if VOXEL_MATERIAL_SINGLE_INDEX_ONLY
#define VOXEL_MATERIAL_ENABLE_R 0
#define VOXEL_MATERIAL_ENABLE_G 0
#define VOXEL_MATERIAL_ENABLE_B 0
#define VOXEL_MATERIAL_ENABLE_A 1
#define VOXEL_MATERIAL_ENABLE_UV0 0
#define VOXEL_MATERIAL_ENABLE_UV1 0
#define VOXEL_MATERIAL_ENABLE_UV2 0
#define VOXEL_MATERIAL_ENABLE_UV3 0
#endif

// Quick way to disable all channels but the ones used to store the color
#ifndef VOXEL_MATERIAL_COLOR_ONLY
#define VOXEL_MATERIAL_COLOR_ONLY 0
#endif

#if VOXEL_MATERIAL_COLOR_ONLY
#define VOXEL_MATERIAL_ENABLE_R 1
#define VOXEL_MATERIAL_ENABLE_G 1
#define VOXEL_MATERIAL_ENABLE_B 1
#define VOXEL_MATERIAL_ENABLE_A 1
#define VOXEL_MATERIAL_ENABLE_UV0 0
#define VOXEL_MATERIAL_ENABLE_UV1 0
#define VOXEL_MATERIAL_ENABLE_UV2 0
#define VOXEL_MATERIAL_ENABLE_UV3 0
#endif

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