// Copyright 2021 Phyronnaz

#include "VoxelQueuedWork.h"

#if ENABLE_VOXEL_QUEUED_WORK_CHECKS
IVoxelQueuedWork::FCheckData IVoxelQueuedWork::CheckData;

void IVoxelQueuedWork::Register()
{
	FScopeLock Lock(&CheckData.Section);
	
	ensure(!CheckData.ValidWorks.Contains(this));
	CheckData.ValidWorks.Add(this);

	CheckData.WorkNames.FindOrAdd(this).Add(Name);
}

void IVoxelQueuedWork::Unregister()
{
	FScopeLock Lock(&CheckData.Section);
	
	ensure(CheckData.ValidWorks.Contains(this));
	CheckData.ValidWorks.Remove(this);
}

void IVoxelQueuedWork::CheckIsValidLowLevel() const
{
	if (!this)
	{
		LOG_VOXEL(Fatal, TEXT("nullptr async work"));
	}
	
	FScopeLock Lock(&CheckData.Section);
	
	if (CheckData.ValidWorks.Contains(this))
	{
		return;
	}

	TArray<FString> StringNames;

	const TArray<FName>* Names = CheckData.WorkNames.Find(this);
	if (Names)
	{
		for (FName It : *Names)
		{
			StringNames.Add(It.ToString());
		}
	}
	
	LOG_VOXEL(Fatal, TEXT("Invalid async work pointer accessed. Names: %s"), *FString::Join(StringNames, TEXT(",")));
}
#endif