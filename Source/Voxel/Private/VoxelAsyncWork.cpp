// Copyright 2020 Phyronnaz

#include "VoxelAsyncWork.h"
#include "VoxelMinimal.h"
#include "HAL/Event.h"
#include "VoxelUtilities/VoxelStatsUtilities.h"

FVoxelAsyncWork::~FVoxelAsyncWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	// DO NOT call WaitForDoThreadedWorkToExit here, as the child class destructor has already be run
	// It's too late to wait

	if (!IsDone())
	{
		LOG_VOXEL(Fatal, TEXT("VoxelAsyncWork %s is being deleted while still in the thread pool!"), *Name.ToString());
	}
	if (DoneSection.IsLocked.GetValue() != 0)
	{
		LOG_VOXEL(Fatal, TEXT("VoxelAsyncWork %s is being deleted while still in DoThreadedWork!"), *Name.ToString());
	}
}

void FVoxelAsyncWork::DoThreadedWork()
{
	VOXEL_ASYNC_VERBOSE_FUNCTION_COUNTER();
	
	check(!IsDone());

	if (!IsCanceled())
	{
		VOXEL_SCOPE_COUNTER_FORMAT("DoWork: %s", *Name.ToString());
		DoWork();
	}

	DoneSection.Lock();

	IsDoneCounter.Increment();

	if (!IsCanceled())
	{
		VOXEL_ASYNC_VERBOSE_SCOPE_COUNTER("PostDoWork");
		check(IsDone());
		PostDoWork();
	}

	if (bAutodelete)
	{
		DoneSection.Unlock();
		delete this;
		return;
	}

	DoneSection.Unlock();
	// Might be deleted right after this
}

void FVoxelAsyncWork::Abandon()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(!IsDone());

	DoneSection.Lock();

	IsDoneCounter.Increment();
	WasAbandonedCounter.Increment();
	
	if (bAutodelete)
	{
		DoneSection.Unlock();
		delete this;
	}
	else
	{
		DoneSection.Unlock();
	}
}

bool FVoxelAsyncWork::CancelAndAutodelete()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	DoneSection.Lock();
	
	check(!bAutodelete);

	bAutodelete = true;
	CanceledCounter.Increment();

	if (IsDone())
	{
		DoneSection.Unlock();
		delete this;
		return true;
	}
	else
	{
		DoneSection.Unlock();
		return false;
	}
}

void FVoxelAsyncWork::WaitForDoThreadedWorkToExit()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	DoneSection.Lock();
	DoneSection.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelAsyncWorkWithWait::FVoxelAsyncWorkWithWait(FName Name, double PriorityDuration, bool bAutoDelete)
	: FVoxelAsyncWork(Name, PriorityDuration, bAutoDelete)
{
	DoneEvent = FPlatformProcess::GetSynchEventFromPool(true);
	DoneEvent->Reset();
}

FVoxelAsyncWorkWithWait::~FVoxelAsyncWorkWithWait()
{
	if (DoneEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
		DoneEvent = nullptr;
	}
}

void FVoxelAsyncWorkWithWait::PostDoWork()
{
	PostDoWorkBeforeTrigger();
	DoneEvent->Trigger();
}

void FVoxelAsyncWorkWithWait::WaitForCompletion()
{
	DoneEvent->Wait();
	// Else the thread that called WaitForCompletion might delete us before DoThreadedWork finishes
	WaitForDoThreadedWorkToExit();
	check(IsDone());
}
