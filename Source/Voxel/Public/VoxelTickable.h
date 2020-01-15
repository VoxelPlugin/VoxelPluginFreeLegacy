// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

class FVoxelTickable : public FTickableGameObject
{
public:
	virtual ~FVoxelTickable()
	{
		ensure(!bShouldTick);
	}
	
	virtual bool IsTickable() const final override
	{
		return bShouldTick;
	}
	virtual TStatId GetStatId() const final override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FVoxelTickable, STATGROUP_Tickables);
	}

	void StopTicking()
	{
		ensure(IsInGameThread());
		ensure(bShouldTick);
		bShouldTick = false;
	}
	void ResumeTicking()
	{
		ensure(IsInGameThread());
		ensure(!bShouldTick);
		bShouldTick = true;
	}
	bool IsTicking() const
	{
		ensure(IsInGameThread());
		return bShouldTick;
	}
	
private:
	bool bShouldTick = true;
};