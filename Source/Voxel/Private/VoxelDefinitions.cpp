// Copyright 2019 Phyronnaz

#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelValue.h"
#include "IVoxelPool.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelCustomVersion.h"

#include "Logging/LogMacros.h"
#include "Serialization/CustomVersion.h"

DEFINE_LOG_CATEGORY(LogVoxel);

// +/- 1000: prevents integers overflow
FIntBox const FIntBox::Infinite = FIntBox(FIntVector(MIN_int32 + 1000, MIN_int32 + 1000, MIN_int32 + 1000), FIntVector(MAX_int32 - 1000, MAX_int32 - 1000, MAX_int32 - 1000));

FVoxelValue const FVoxelValue::Full = FVoxelValue(-1);
FVoxelValue const FVoxelValue::Empty = FVoxelValue(1);

TSharedPtr<IVoxelPool> IVoxelPool::Global = nullptr;
TArray<FVoxelPlaceableItem*> FVoxelPlaceableItem::DefaultItems;

const FGuid FVoxelCustomVersion::GUID(0x07949d97, 0x082f2e4f, 0x0593de63, 0x1164cfc5);
// Register the custom version with core
FCustomVersionRegistration GRegisterVoxelCustomVersion(FVoxelCustomVersion::GUID, FVoxelCustomVersion::LatestVersion, TEXT("VoxelVer"));