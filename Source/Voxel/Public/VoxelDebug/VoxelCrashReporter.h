// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class VOXEL_API IVoxelCrashReporter
{
public:
	virtual ~IVoxelCrashReporter() = default;
	virtual bool ShowError(const FString& Error, bool bCanIgnore) = 0;
};

class VOXEL_API FVoxelCrashReporter
{
public:
	static TSharedPtr<IVoxelCrashReporter> CrashReporter;

	static void ShowError(const FString& Error, bool bCanIgnore = true);
	static void ShowWorldGeneratorError();
	static void ResetIgnoreMessages();
	
private:
	static bool bIgnoreMessages;
};