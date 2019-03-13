// Copyright 2019 Phyronnaz

#pragma once

#include "Stats/Stats.h"
#include "Logging/LogMacros.h"

DECLARE_STATS_GROUP(TEXT("Voxel"), STATGROUP_Voxel, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Memory"), STATGROUP_VoxelMemory, STATCAT_Advanced);

VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Verbose, All);

#define VOXELPLUGIN_VERSION TEXT("0")

#ifndef VOXEL_DEBUG
#define VOXEL_DEBUG 0
#endif // !VOXEL_DEBUG

#if VOXEL_DEBUG
#define checkVoxelSlow(x) check(x)
#else
#define checkVoxelSlow(x)
#endif

#if PLATFORM_WINDOWS
#define VOXEL_THREADLOCAL thread_local
#else
#define VOXEL_THREADLOCAL static // Non trivial destructors don't work with clang -libc++
#endif

#define DO_THREADSAFE_CHECKS VOXEL_DEBUG
#define ENABLE_LOCKER_NAME !UE_BUILD_SHIPPING

#if DO_THREADSAFE_CHECKS
#define ensureThreadSafe(...) ensure(__VA_ARGS__)
#else
#define ensureThreadSafe(...)
#endif

#define ENABLE_TESSELLATION (!PLATFORM_ANDROID && !PLATFORM_SWITCH)
#define ENABLE_OPTIMIZE_INDICES PLATFORM_WINDOWS

///////////////////////////////////////////////////////////////////////////////

namespace VoxelGlobalsUtils
{
	template<typename T>
	inline constexpr bool IsPowerOfTwo(T Value)
	{
		return ((Value & (Value - 1)) == (T)0);
	}

	inline constexpr int IntLog2(int X)
	{
		int Exp = -1;
		while (X)
		{
			X >>= 1;
			++Exp;
		}
		return Exp;
	}

	inline constexpr int DataOctreeDepthDiff(int DataChunkSize, int ChunkSize)
	{
		if (DataChunkSize <= ChunkSize)
		{
			return IntLog2(ChunkSize / DataChunkSize); // Depth must be higher if chunk size is smaller
		}
		else
		{
			return -IntLog2(DataChunkSize / ChunkSize);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

#define CHUNK_SIZE 32 // size of rendering chunks
static_assert(VoxelGlobalsUtils::IsPowerOfTwo(CHUNK_SIZE), "CHUNK_SIZE must be a power of 2");

#define VOXEL_CELL_SIZE 16 // size of data chunks
static_assert(VoxelGlobalsUtils::IsPowerOfTwo(VOXEL_CELL_SIZE), "VOXEL_CELL_SIZE must be a power of 2");

#define DATA_OCTREE_DEPTH_DIFF VoxelGlobalsUtils::DataOctreeDepthDiff(VOXEL_CELL_SIZE, CHUNK_SIZE) 

#define VOXEL_CELL_COUNT (VOXEL_CELL_SIZE * VOXEL_CELL_SIZE * VOXEL_CELL_SIZE)

#define MAX_WORLD_DEPTH 26
static_assert(MAX_WORLD_DEPTH % 2 == 0, "MAX_WORLD_DEPTH must be a multiple of 2");

#define MAX_PLACEABLE_ITEMS_PER_OCTREE 4

// Inclusive
#define MAX_LOD_USED_FOR_CACHE 1

#define EDIT_TOOLS_LOCK_TIMEOUT 0.0001

///////////////////////////////////////////////////////////////////////////////

#define DISABLE_VOXELINDEX 0
#define ENABLE_VOXELCOLORS 1
#define ENABLE_VOXELACTORS 1
#define ENABLE_VOXELGRASS 1

enum EVoxelConfigFlags : uint32
{
	EnableVoxelColors        = 0x01,
	EnableVoxelSpawnedActors = 0x02,
	EnableVoxelGrass         = 0x04,
	DisableIndex             = 0x10
}; 
inline constexpr uint32 GetVoxelConfigFlags()
{
	return
		EnableVoxelColors * ENABLE_VOXELCOLORS        +
		EnableVoxelSpawnedActors * ENABLE_VOXELACTORS +
		EnableVoxelGrass * ENABLE_VOXELGRASS          +
		DisableIndex * DISABLE_VOXELINDEX
		;
}

///////////////////////////////////////////////////////////////////////////////

using FVoxelCellIndex = uint16;
static_assert(VOXEL_CELL_COUNT < TNumericLimits<FVoxelCellIndex>::Max(), "CellIndex type is too small");