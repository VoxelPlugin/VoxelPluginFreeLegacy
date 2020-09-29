// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelUtilities/VoxelDataItemUtilities.h"

namespace FVoxelNodeFunctions
{
	inline v_flt GetDataItemDistance(
		const FVoxelPlaceableItemHolder& ItemHolder,
		v_flt X, v_flt Y, v_flt Z,
		v_flt Smoothness,
		v_flt Default,
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode)
	{
		return FVoxelUtilities::GetDataItemDistance(ItemHolder, X, Y, Z, Smoothness, Default, Mask, CombineMode);
	}

	inline TVoxelRange<v_flt> GetDataItemDistance(
		const FVoxelPlaceableItemHolder& ItemHolder, 
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, 
		TVoxelRange<v_flt> Smoothness, 
		TVoxelRange<v_flt> Default, 
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode)
	{
		return FVoxelUtilities::GetDataItemDistanceRange(ItemHolder, X, Y, Z, Smoothness, Default, Mask, CombineMode);
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline v_flt GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, v_flt X, v_flt Y, v_flt Z, v_flt Smoothness, v_flt Default)
	{
		return GetDataItemDistance(ItemHolder, X, Y, Z, Smoothness, Default, uint32(-1), EVoxelDataItemCombineMode::Min);
	}
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TVoxelRange<v_flt> GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, TVoxelRange<v_flt> Smoothness, TVoxelRange<v_flt> Default)
	{
		return GetDataItemDistance(ItemHolder, X, Y, Z, Smoothness, Default, uint32(-1), EVoxelDataItemCombineMode::Min);
	}
}