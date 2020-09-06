// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "NodeFunctions/VoxelNodeFunctions.h"
#include "VoxelUtilities/VoxelSDFUtilities.h"
#include "VoxelNodes/VoxelPlaceableItemsNodes.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.inl"

namespace FVoxelNodeFunctions
{
	inline v_flt GetDataItemDistance(
		const FVoxelPlaceableItemHolder& ItemHolder,
		v_flt X, v_flt Y, v_flt Z,
		v_flt Smoothness,
		v_flt Default,
		uint32 Mask,
		EVoxelDataItemSampleCombineMode CombineMode)
	{
		const auto ShouldComputeItem = [&](const FVoxelDataItem& Item)
		{
			return (Item.Mask & Mask) && Item.Bounds.ContainsFloat(X, Y, Z);
		};
		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			return Item.WorldGenerator->GetValue(X, Y, Z, 0, Stack);
		};
		
		auto& DataItems = ItemHolder.GetDataItems();
		int32 Index = 0;

		const auto FindNextItem = [&]()
		{
			while (Index < DataItems.Num() && !ShouldComputeItem(*DataItems[Index]))
			{
				Index++;
			}
		};
		
		FindNextItem();

		if (Index == DataItems.Num())
		{
			return Default;
		}
		
		// Note: we can't use Default here else SmoothUnion is messed up
		v_flt BestDistance = GetItemDistance(*DataItems[Index++]);

		for (;;)
		{
			FindNextItem();

			if (Index == DataItems.Num())
			{
				return BestDistance;
			}

			const v_flt Distance = GetItemDistance(*DataItems[Index++]);

			if (CombineMode == EVoxelDataItemSampleCombineMode::Min)
			{
				BestDistance = Smoothness <= 0 ? FMath::Min(Distance, BestDistance) : FVoxelSDFUtilities::opSmoothUnion(Distance, BestDistance, Smoothness);
			}
			else if (CombineMode == EVoxelDataItemSampleCombineMode::Max)
			{
				BestDistance = Smoothness <= 0 ? FMath::Max(Distance, BestDistance) : FVoxelSDFUtilities::opSmoothIntersection(Distance, BestDistance, Smoothness);
			}
			else
			{
				ensureVoxelSlow(CombineMode == EVoxelDataItemSampleCombineMode::Sum);
				BestDistance += Distance;
			}
		}
	}

	inline TVoxelRange<v_flt> GetDataItemDistance(
		const FVoxelPlaceableItemHolder& ItemHolder, 
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, 
		TVoxelRange<v_flt> Smoothness, 
		TVoxelRange<v_flt> Default, 
		uint32 Mask,
		EVoxelDataItemSampleCombineMode CombineMode)
	{
		const FVoxelIntBox Bounds = FVoxelNodeFunctions::BoundsFromRanges(X, Y, Z);
		
		const auto ShouldComputeItem = [&](const FVoxelDataItem& Item)
		{
			return (Item.Mask & Mask) && Item.Bounds.Intersect(Bounds);
		};
		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			auto Range = Item.WorldGenerator->GetValueRange(Bounds, 0, Stack);

			if (!Item.Bounds.Contains(Bounds))
			{
				// We might return Default if we're queried outside of the item bounds
				Range = TVoxelRange<v_flt>::Union(Range, Default);
			}

			return Range;
		};
		
		auto& DataItems = ItemHolder.GetDataItems();
		int32 Index = 0;

		const auto FindNextItem = [&]()
		{
			while (Index < DataItems.Num() && !ShouldComputeItem(*DataItems[Index]))
			{
				Index++;
			}
		};
		
		FindNextItem();

		if (Index == DataItems.Num())
		{
			return Default;
		}
		
		// Note: we can't use Default here else SmoothUnion is messed up
		TVoxelRange<v_flt> BestDistance = GetItemDistance(*DataItems[Index++]);

		for (;;)
		{
			FindNextItem();

			if (Index == DataItems.Num())
			{
				const v_flt Extent = FMath::Max<v_flt>(Smoothness.Max, 0);
				return { BestDistance.Min - Extent, BestDistance.Max + Extent };
			}

			const TVoxelRange<v_flt> Distance = GetItemDistance(*DataItems[Index++]);
			
			if (CombineMode == EVoxelDataItemSampleCombineMode::Min)
			{
				BestDistance = FVoxelRangeUtilities::Min(Distance, BestDistance);
			}
			else if (CombineMode == EVoxelDataItemSampleCombineMode::Max)
			{
				BestDistance = FVoxelRangeUtilities::Max(Distance, BestDistance);
			}
			else
			{
				ensureVoxelSlow(CombineMode == EVoxelDataItemSampleCombineMode::Sum);
				BestDistance += Distance;
			}
		}
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline v_flt GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, v_flt X, v_flt Y, v_flt Z, v_flt Smoothness, v_flt Default)
	{
		return GetDataItemDistance(ItemHolder, X, Y, Z, Smoothness, Default, uint32(-1), EVoxelDataItemSampleCombineMode::Min);
	}
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TVoxelRange<v_flt> GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, TVoxelRange<v_flt> Smoothness, TVoxelRange<v_flt> Default)
	{
		return GetDataItemDistance(ItemHolder, X, Y, Z, Smoothness, Default, uint32(-1), EVoxelDataItemSampleCombineMode::Min);
	}
}