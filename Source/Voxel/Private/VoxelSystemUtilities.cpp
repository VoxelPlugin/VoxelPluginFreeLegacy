// Copyright 2020 Phyronnaz

#include "VoxelUtilities/VoxelSystemUtilities.h"
#include "Containers/Ticker.h"

void FVoxelUtilities::DelayedCall(TFunction<void()> Call, float Delay)
{
	check(IsInGameThread());
	
	FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([=](float)
	{
		Call();
		return false;
	}), Delay);
}