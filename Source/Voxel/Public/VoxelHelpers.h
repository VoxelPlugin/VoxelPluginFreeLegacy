// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelStats.h"

class FVoxelScopeLockWithStats
{
public:
	FVoxelScopeLockWithStats(FCriticalSection& InSynchObject)
		: SynchObject(InSynchObject)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Lock");
		SynchObject.Lock();
	}
	~FVoxelScopeLockWithStats()
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Unlock");
		SynchObject.Unlock();
	}

private:
	FCriticalSection& SynchObject;
};
