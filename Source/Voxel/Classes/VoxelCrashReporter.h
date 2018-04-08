// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class VOXEL_API IVoxelCrashReporter
{
public:
	virtual bool ShowError(const FString& Error) = 0;
};

class VOXEL_API FVoxelCrashReporter
{
public:
	static TSharedPtr<IVoxelCrashReporter> CrashReporter;

	static void ShowError(const FString& Error);
	static void ShowApproximationError();
	
	static bool bIgnoreMessages;
};