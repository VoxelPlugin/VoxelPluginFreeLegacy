// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformTime.h"

struct FVoxelStatEntry
{
	uint64 Cycles = 0;
	uint64 Count = 0;

	double Seconds() const { return FPlatformTime::ToSeconds64(Cycles); }
	FVoxelStatEntry& operator+=(const FVoxelStatEntry& Other)
	{
		Cycles += Other.Cycles;
		Count += Other.Count;
		return *this;
	}
};
struct FVoxelScopedStat
{
	const uint64 StartCycle = FPlatformTime::Cycles64();
	FVoxelStatEntry& Entry;

	FORCEINLINE FVoxelScopedStat(FVoxelStatEntry& Entry, uint64 Count)
		: Entry(Entry)
	{
		Entry.Count += Count;
	}
	FORCEINLINE ~FVoxelScopedStat()
	{
		Entry.Cycles += FPlatformTime::Cycles64() - StartCycle;
	}
};

#define VOXEL_SCOPED_STAT(Time, Count) FVoxelScopedStat PREPROCESSOR_JOIN(__VoxelScopedTime, __LINE__)(Time, Count)
#define VOXEL_INLINE_STAT(Time, Count, Expr) [&]() { FVoxelScopedStat LocalScope(Time, Count); return Expr; }()
