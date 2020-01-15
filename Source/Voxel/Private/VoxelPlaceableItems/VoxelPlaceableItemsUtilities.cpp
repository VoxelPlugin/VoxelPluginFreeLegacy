// Copyright 2020 Phyronnaz

#include "VoxelPlaceableItems/VoxelPlaceableItemsUtilities.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorld.h"
#include "FastNoise.h"
#include "VoxelMessages.h"

void UVoxelPlaceableItemsUtilities::AddWorms(
	AVoxelWorld* World,
	float Radius,
	int32 Seed,
	FVector RotationAmplitude,
	FVector NoiseDir,
	float NoiseSegmentLength,
	FVector Start,
	FVector InitialDir,
	float VoxelSegmentLength,
	int32 NumSegments, 
	float SplitProbability,
	float SplitProbabilityGain,
	int32 BranchMeanSize,
	int32 BranchSizeVariation)
{
	VOXEL_PRO_ONLY_VOID();
}