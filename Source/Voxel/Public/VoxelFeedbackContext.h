// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/ScopedSlowTask.h"

// Use this to set the plugin to use a custom feedback context for progress bars/slow tasks
VOXEL_API void SetVoxelFeedbackContext(FFeedbackContext& FeedbackContext);

struct VOXEL_API FVoxelScopedSlowTask : FScopedSlowTask
{
	explicit FVoxelScopedSlowTask(float InAmountOfWork, const FText& InDefaultMessage = FText(), bool bInEnabled = true);
};

struct FVoxelScopedFeedbackContext
{
	explicit FVoxelScopedFeedbackContext(FFeedbackContext& FeedbackContext)
	{
		SetVoxelFeedbackContext(FeedbackContext);
	}
	~FVoxelScopedFeedbackContext()
	{
		SetVoxelFeedbackContext(*GWarn);
	}
};