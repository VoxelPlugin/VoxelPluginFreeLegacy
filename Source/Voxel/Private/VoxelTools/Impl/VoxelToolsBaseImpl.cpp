// Copyright 2020 Phyronnaz

#include "VoxelTools/Impl/VoxelToolsBaseImpl.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"

static TAutoConsoleVariable<int32> CVarLogEditToolsTimes(
	TEXT("voxel.tools.LogEditTimes"),
	0,
	TEXT("Log edit tools times"),
	ECVF_Default);

FScopeToolsTimeLogger::~FScopeToolsTimeLogger()
{
	const double EndTime = FPlatformTime::Seconds();
	if (CVarLogEditToolsTimes.GetValueOnAnyThread() != 0)
	{
		const double ElapsedInSeconds = (EndTime - StartTime);
		const double ElapsedInMilliseconds = ElapsedInSeconds * 1000;
		if (NumVoxels < 0)
		{
			LOG_VOXEL(Log, TEXT("%s took %fms"), *FString(Name), ElapsedInMilliseconds);
		}
		else
		{
			LOG_VOXEL(Log, TEXT("%s took %fms for %lld voxels (%f G/s)"), *FString(Name), ElapsedInMilliseconds, NumVoxels, NumVoxels / ElapsedInSeconds / 1e9);
		}
	}
}