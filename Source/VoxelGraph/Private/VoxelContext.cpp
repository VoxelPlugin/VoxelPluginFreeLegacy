// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelContext.h"

const FVoxelContext FVoxelContext::EmptyContext = FVoxelContext(
	0,
	FVoxelItemStack::Empty,
	FTransform::Identity,
	false);

const FVoxelContextRange FVoxelContextRange::EmptyContext = FVoxelContextRange(
	0,
	FVoxelItemStack::Empty,
	FTransform::Identity,
	false,
	FVoxelIntBox());