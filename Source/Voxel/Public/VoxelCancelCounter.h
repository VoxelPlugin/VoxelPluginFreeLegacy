// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSharedPtr.h"

struct FVoxelCancelCounter
{
	const TVoxelSharedRef<FThreadSafeCounter64> Counter;
	const int64 Threshold;

	explicit FVoxelCancelCounter(const TVoxelSharedRef<FThreadSafeCounter64>& Counter)
		: Counter(Counter)
		, Threshold(Counter->GetValue())
	{
	}

	inline bool IsCanceled() const
	{
		return Counter->GetValue() > Threshold;
	}
};