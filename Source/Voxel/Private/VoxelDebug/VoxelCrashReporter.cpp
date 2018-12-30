// Copyright 2018 Phyronnaz

#include "VoxelDebug/VoxelCrashReporter.h"
#include "VoxelLogStatDefinitions.h"

TSharedPtr<IVoxelCrashReporter> FVoxelCrashReporter::CrashReporter;
bool FVoxelCrashReporter::bIgnoreMessages = false;

void FVoxelCrashReporter::ShowError(const FString& InError, bool bCanIgnore)
{
	UE_LOG(LogVoxel, Error, TEXT("Error: %s"), *InError);
	if (CrashReporter.IsValid() && (!bIgnoreMessages || !bCanIgnore))
	{
		bIgnoreMessages = CrashReporter->ShowError(InError, bCanIgnore);
	}
}

void FVoxelCrashReporter::ShowWorldGeneratorError()
{
	ShowError(TEXT("Graph isn't consistent: values for different LODs aren't the same.\nThis can be caused by an unsafe usage of the LOD node, or by float approximations errors."));
}

void FVoxelCrashReporter::ResetIgnoreMessages()
{
	bIgnoreMessages = false;
}

