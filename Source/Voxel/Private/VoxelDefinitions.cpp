// Copyright 2019 Phyronnaz

#include "VoxelGlobals.h"
#include "IntBox.h"
#include "VoxelValue.h"
#include "IVoxelPool.h"

#include "Logging/LogMacros.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

DEFINE_LOG_CATEGORY(LogVoxel);

// +/- 1000: prevents integers overflow
FIntBox const FIntBox::Infinite = FIntBox(FIntVector(MIN_int32 + 1000, MIN_int32 + 1000, MIN_int32 + 1000), FIntVector(MAX_int32 - 1000, MAX_int32 - 1000, MAX_int32 - 1000));

FVoxelValue const FVoxelValue::Full = FVoxelValue(-1);
FVoxelValue const FVoxelValue::Empty = FVoxelValue(1);

TSharedPtr<IVoxelPool> IVoxelPool::Global = nullptr;
uint64 FVoxelPlaceableItem::StaticUniqueId = 0;