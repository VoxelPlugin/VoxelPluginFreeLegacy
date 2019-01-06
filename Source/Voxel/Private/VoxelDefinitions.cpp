// Copyright 2019 Phyronnaz

#include "IntBox.h"
#include "VoxelValue.h"
#include "VoxelGraph/VoxelNodeDefinitions.h"

// +/- 1000: prevents integers overflow
FIntBox const FIntBox::Infinite = FIntBox(FIntVector(MIN_int32 + 1000, MIN_int32 + 1000, MIN_int32 + 1000), FIntVector(MAX_int32 - 1000, MAX_int32 - 1000, MAX_int32 - 1000));

FVoxelValue const FVoxelValue::Full = FVoxelValue(-1);
FVoxelValue const FVoxelValue::Empty = FVoxelValue(1);

const FVoxelPlaceableItemHolder FVoxelContext::StaticItem;

