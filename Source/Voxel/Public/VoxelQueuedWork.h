// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/IQueuedWork.h"

class IVoxelQueuedWork : public IQueuedWork
{
public:
	// Used for performance reporting and debugging
	const FName Name;
	// Number of seconds a priority can be cached before being recomputed
	const double PriorityDuration;

	FORCEINLINE IVoxelQueuedWork(FName Name, double PriorityDuration)
		: Name(Name)
		, PriorityDuration(PriorityDuration)
	{
	}

	IVoxelQueuedWork(const IVoxelQueuedWork&) = delete;
	IVoxelQueuedWork& operator=(const IVoxelQueuedWork&) = delete;
	
	// Called to determine which thread to process next
	// Voxel works are usually quite long, so it's worth it to compute all the priorities
	// Must be thread safe
	virtual uint32 GetPriority() const = 0;
};