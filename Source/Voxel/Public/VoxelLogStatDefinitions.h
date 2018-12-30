// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("Voxel"), STATGROUP_Voxel, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("Voxel Memory"), STATGROUP_VoxelMemory, STATCAT_Advanced);

DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Verbose, All);

#if 0 // To disable stats
#undef DECLARE_CYCLE_STAT
#define DECLARE_CYCLE_STAT(...) 

#undef SCOPE_CYCLE_COUNTER
#define SCOPE_CYCLE_COUNTER(...) 

#undef DECLARE_MEMORY_STAT
#define DECLARE_MEMORY_STAT(...) 

#undef DECLARE_DWORD_ACCUMULATOR_STAT
#define DECLARE_DWORD_ACCUMULATOR_STAT(...) 

#undef INC_DWORD_STAT_BY
#define INC_DWORD_STAT_BY(...) 

#undef INC_MEMORY_STAT_BY
#define INC_MEMORY_STAT_BY(...) 

#undef DEC_DWORD_STAT_BY
#define DEC_DWORD_STAT_BY(...) 

#undef DEC_MEMORY_STAT_BY
#define DEC_MEMORY_STAT_BY(...) 
#endif