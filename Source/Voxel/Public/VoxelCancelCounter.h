// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"

class FVoxelCancelCounter
{
public:
	FVoxelCancelCounter() = default;

	void Cancel() const
	{
		Counter->Increment();
	}
	
private:
	TVoxelSharedRef<FThreadSafeCounter> Counter = MakeVoxelShared<FThreadSafeCounter>();

	friend class FVoxelCancelTracker;
};

class FVoxelCancelTracker
{
public:
	FVoxelCancelTracker()
		: Counter(MakeVoxelShared<FThreadSafeCounter>())
		, Threshold(-1)
	{
	}
	FVoxelCancelTracker(const FVoxelCancelCounter& InCounter)
		: Counter(InCounter.Counter)
		, Threshold(Counter->GetValue())
	{
	}

	bool IsCanceled() const
	{
		return Counter->GetValue() > Threshold;
	}

private:
	TVoxelSharedRef<FThreadSafeCounter> Counter;
	int64 Threshold;
};