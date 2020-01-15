// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataAccelerator.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarCacheSize(
	TEXT("voxel.data.DataAccelerator.CacheSize"),
	8,
	TEXT("Size of the data accelerator cache"),
	ECVF_Default);
static TAutoConsoleVariable<int32> CVarUseAcceleratorMap(
	TEXT("voxel.data.DataAccelerator.UseMap"),
	1,
	TEXT("Whether to cache the leaves in a map"),
	ECVF_Default);
static TAutoConsoleVariable<int32> CVarShowStats(
	TEXT("voxel.data.DataAccelerator.LogStats"),
	0,
	TEXT("Log stats about accelerators hit/misses"),
	ECVF_Default);

int32 FVoxelDataAcceleratorParameters::GetDefaultCacheSize()
{
	return CVarCacheSize.GetValueOnAnyThread();
}
bool FVoxelDataAcceleratorParameters::GetUseAcceleratorMap()
{
	return CVarUseAcceleratorMap.GetValueOnAnyThread() != 0;
}
bool FVoxelDataAcceleratorParameters::GetShowStats()
{
	return CVarShowStats.GetValueOnAnyThread() != 0;
}