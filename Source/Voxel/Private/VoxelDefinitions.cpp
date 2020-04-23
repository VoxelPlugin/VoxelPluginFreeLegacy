// Copyright 2020 Phyronnaz

#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelCustomVersion.h"
#include "VoxelItemStack.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

#include "Logging/LogMacros.h"
#include "Serialization/CustomVersion.h"

DEFINE_LOG_CATEGORY(LogVoxel);

#if ENABLE_VOXEL_MEMORY_STATS
TMap<const TCHAR*, FVoxelMemoryCounterRef>& GetVoxelMemoryCounters()
{
	static TMap<const TCHAR*, FVoxelMemoryCounterRef> GVoxelMemoryCounters;
	return GVoxelMemoryCounters;
}

FVoxelMemoryCounterStaticRef::FVoxelMemoryCounterStaticRef(const TCHAR* Name, const FVoxelMemoryCounterRef& Ref)
{
	GetVoxelMemoryCounters().Add(Name, Ref);
}
#endif

DEFINE_VOXEL_MEMORY_STAT(STAT_TotalVoxelMemory);

static FFeedbackContext* GVoxelFeedbackContext = nullptr;

void SetVoxelFeedbackContext(class FFeedbackContext& FeedbackContext)
{
	GVoxelFeedbackContext = &FeedbackContext;
}

FVoxelScopedSlowTask::FVoxelScopedSlowTask(float InAmountOfWork, const FText& InDefaultMessage, bool bInEnabled)
	: FScopedSlowTask(InAmountOfWork, InDefaultMessage, bInEnabled, GVoxelFeedbackContext ? *GVoxelFeedbackContext : *GWarn)
{

}

// +/- 1024: prevents integers overflow
FIntBox const FIntBox::Infinite = FIntBox(FIntVector(MIN_int32 + 1024), FIntVector(MAX_int32 - 1024));

FVoxelItemStack FVoxelItemStack::Empty = FVoxelItemStack(FVoxelPlaceableItemHolder::Empty);

const FGuid FVoxelCustomVersion::GUID(0x07949d97, 0x082f2e4f, 0x0593de63, 0x1164cfc5);
// Register the custom version with core
FCustomVersionRegistration GRegisterVoxelCustomVersion(FVoxelCustomVersion::GUID, FVoxelCustomVersion::LatestVersion, TEXT("VoxelVer"));

const FVoxelVector FVoxelVector::ZeroVector(0.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::OneVector(1.0f, 1.0f, 1.0f);
const FVoxelVector FVoxelVector::UpVector(0.0f, 0.0f, 1.0f);
const FVoxelVector FVoxelVector::DownVector(0.0f, 0.0f, -1.0f);
const FVoxelVector FVoxelVector::ForwardVector(1.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::BackwardVector(-1.0f, 0.0f, 0.0f);
const FVoxelVector FVoxelVector::RightVector(0.0f, 1.0f, 0.0f);
const FVoxelVector FVoxelVector::LeftVector(0.0f, -1.0f, 0.0f);