// Copyright 2020 Phyronnaz

#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelCustomVersion.h"
#include "VoxelItemStack.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

#include "Logging/LogMacros.h"
#include "Serialization/CustomVersion.h"

DEFINE_LOG_CATEGORY(LogVoxel);

// +/- 1024: prevents integers overflow
FIntBox const FIntBox::Infinite = FIntBox(FIntVector(MIN_int32 + 1024), FIntVector(MAX_int32 - 1024));

FVoxelItemStack FVoxelItemStack::Empty = FVoxelItemStack(FVoxelPlaceableItemHolder::Empty);

const FGuid FVoxelCustomVersion::GUID(0x07949d97, 0x082f2e4f, 0x0593de63, 0x1164cfc5);
// Register the custom version with core
FCustomVersionRegistration GRegisterVoxelCustomVersion(FVoxelCustomVersion::GUID, FVoxelCustomVersion::LatestVersion, TEXT("VoxelVer"));