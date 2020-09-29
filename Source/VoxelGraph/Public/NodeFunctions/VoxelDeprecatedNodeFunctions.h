// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelGraphGlobals.h"
#include "VoxelUtilities/VoxelRangeUtilities.h"

class FVoxelPlaceableItemHolder;

DEPRECATED_VOXEL_GRAPH_FUNCTION()
typedef EVoxelDataItemCombineMode EVoxelDataItemSampleCombineMode;

namespace FVoxelNodeFunctions
{
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline v_flt GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, v_flt X, v_flt Y, v_flt Z)
	{
		return 0;
	}
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TVoxelRange<v_flt> GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, const TVoxelRange<v_flt>& X, const TVoxelRange<v_flt>& Y, const TVoxelRange<v_flt>& Z)
	{
		return 0;
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline FVoxelIntBox BoundsFromRanges(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z)
	{
		return FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	}

}