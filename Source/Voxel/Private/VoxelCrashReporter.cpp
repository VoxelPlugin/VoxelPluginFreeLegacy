// Copyright 2018 Phyronnaz

#include "VoxelCrashReporter.h"
#include "VoxelPrivate.h"

TSharedPtr<IVoxelCrashReporter> FVoxelCrashReporter::CrashReporter;
bool FVoxelCrashReporter::bIgnoreMessages = false;

void FVoxelCrashReporter::ShowError(const FString& InError)
{
	UE_LOG(LogVoxel, Error, TEXT("Error: %s"), *InError);
	if (CrashReporter.IsValid() && !bIgnoreMessages)
	{
		bIgnoreMessages = CrashReporter->ShowError(InError + TEXT("\n\nHide future errors?"));
	}
}

void FVoxelCrashReporter::ShowApproximationError()
{
	ShowError(TEXT("Approximation error\nTry reconfiguring your graph\n\nNote: Using more Integers can reduce approximation errors"));
}

