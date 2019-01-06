// Copyright 2019 Phyronnaz

#pragma once

#include <array>
#include "HAL/Platform.h"

#ifndef VOXEL_DEBUG
#define VOXEL_DEBUG 0
#endif // !VOXEL_DEBUG

#if VOXEL_DEBUG
#define checkVoxelSlow(x) check(x)
#else
#define checkVoxelSlow(x)
#endif

#define ENABLE_VOXELGRAPH_CHECKS 0

#if ENABLE_VOXELGRAPH_CHECKS
#define checkVoxelGraph(...) check(__VA_ARGS__)
#else
#define checkVoxelGraph(...)
#endif

#if PLATFORM_WINDOWS
#define VOXEL_THREADLOCAL thread_local
#else
#define VOXEL_THREADLOCAL static // Non trivial destructors don't work with clang -libc++
#endif

#define ENABLE_TESSELLATION !PLATFORM_ANDROID
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

///////////////////////////////////////////////////////////////////////////////

#define MAX_VOXELNODE_PINS 64
#define MAX_VOXELGRAPH_VARIABLES 0xFFFFF
#define MAX_VOXELFUNCTION_ARGS 32
#define MAX_VOXELGRAPH_OUTPUTS 64
static_assert(MAX_VOXELGRAPH_OUTPUTS < MAX_uint8, "");
static_assert(MAX_VOXELGRAPH_OUTPUTS <= sizeof(uint64) * 8, "");

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
	EnableRGBA               = 0x08,
	DisableIndex             = 0x10
}; 
inline uint32 GetVoxelConfigFlags()
{
	return
		EnableVoxelColors * ENABLE_VOXELCOLORS        +
		EnableVoxelSpawnedActors * ENABLE_VOXELACTORS +
		EnableVoxelGrass * ENABLE_VOXELGRASS          +
		EnableRGBA                                    +
		DisableIndex * DISABLE_VOXELINDEX
		;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
using TVoxelBuffer = std::array<T, VOXEL_CELL_COUNT>;

using FVoxelCellIndex = uint16;
static_assert(VOXEL_CELL_COUNT < std::numeric_limits<FVoxelCellIndex>::max(), "CellIndex type is too small");