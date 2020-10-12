// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"

class FVoxelCancelCounter
{
public:
	explicit FVoxelCancelCounter(const TVoxelSharedRef<FThreadSafeCounter64>& Counter)
		: Counter(Counter)
		, Threshold(Counter->GetValue())
	{
	}

	bool IsCanceled() const
	{
		return Counter->GetValue() > Threshold;
	}

private:
	const TVoxelSharedRef<FThreadSafeCounter64> Counter;
	const int64 Threshold;
};