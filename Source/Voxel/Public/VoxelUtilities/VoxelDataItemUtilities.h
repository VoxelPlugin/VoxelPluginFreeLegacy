// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelItemStack.h"
#include "VoxelUtilities/VoxelSDFUtilities.h"
#include "VoxelUtilities/VoxelRangeUtilities.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelGenerators/VoxelGeneratorInstance.inl"

namespace FVoxelUtilities
{
	/**
	 * Get the distance to data items
	 * See https://wiki.voxelplugin.com/Voxel_Data_Items
	 * @param	ItemHolder		The item holder passed to the generator
	 * @param	X				The current X coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Y				The current Y coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Z				The current Z coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Smoothness		The smoothness of the union/intersection. Should be >= 0
	 * @param	Default			The default value to return if no 
	 * @param	Mask			Use uint32(-1) to match any items
	 * @param	CombineMode		How to combine data items
	 * @param	GeneratorValue	If set, Default will be ignored and GeneratorValue returned instead. GeneratorValue will be combined with the data item distances.
	 * @return	The resulting distance
	 */
	template<bool bInvertDataItemDistances = false>
	inline v_flt GetDataItemDistance(
		const FVoxelPlaceableItemHolder& ItemHolder,
		v_flt X, v_flt Y, v_flt Z,
		v_flt Smoothness,
		v_flt Default,
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode,
		const v_flt* GeneratorValue = nullptr)
	{
		const auto ShouldComputeItem = [&](const FVoxelDataItem& Item)
		{
			return (Item.Mask & Mask) && Item.Bounds.ContainsFloat(X, Y, Z);
		};
		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			return Item.Generator->GetValue(X, Y, Z, 0, Stack) * (bInvertDataItemDistances ? -1 : 1);
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
			if (GeneratorValue)
			{
				return *GeneratorValue;
			}
			else
			{
				return Default;
			}
		}
		
		// Note: we can't use Default here else SmoothUnion is messed up
		v_flt BestDistance = GeneratorValue ? *GeneratorValue : GetItemDistance(*DataItems[Index++]);

		for (;;)
		{
			FindNextItem();

			if (Index == DataItems.Num())
			{
				return BestDistance;
			}

			const v_flt Distance = GetItemDistance(*DataItems[Index++]);

			if (CombineMode == EVoxelDataItemCombineMode::Min)
			{
				BestDistance = Smoothness <= 0 ? FMath::Min(Distance, BestDistance) : FVoxelSDFUtilities::opSmoothUnion(Distance, BestDistance, Smoothness);
			}
			else if (CombineMode == EVoxelDataItemCombineMode::Max)
			{
				BestDistance = Smoothness <= 0 ? FMath::Max(Distance, BestDistance) : FVoxelSDFUtilities::opSmoothIntersection(Distance, BestDistance, Smoothness);
			}
			else
			{
				ensureVoxelSlow(CombineMode == EVoxelDataItemCombineMode::Sum);
				BestDistance += Distance;
			}
		}
	}

	// Useful for GetValueRange
	template<bool bInvertDataItemDistances = false>
	inline TVoxelRange<v_flt> GetDataItemDistanceRange(
		const FVoxelPlaceableItemHolder& ItemHolder, 
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, 
		TVoxelRange<v_flt> Smoothness, 
		TVoxelRange<v_flt> Default, 
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode,
		const TVoxelRange<v_flt>* GeneratorValue = nullptr)
	{
		const FVoxelIntBox Bounds = FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
		
		const auto ShouldComputeItem = [&](const FVoxelDataItem& Item)
		{
			return (Item.Mask & Mask) && Item.Bounds.Intersect(Bounds);
		};
		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			auto Range = Item.Generator->GetValueRange(Bounds, 0, Stack);
			if (bInvertDataItemDistances)
			{
				Range = -Range;
			}

			if (!GeneratorValue && !Item.Bounds.Contains(Bounds))
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
			if (GeneratorValue)
			{
				return *GeneratorValue;
			}
			else
			{
				return Default;
			}
		}
		
		// Note: we can't use Default here else SmoothUnion is messed up
		TVoxelRange<v_flt> BestDistance = GeneratorValue ? *GeneratorValue : GetItemDistance(*DataItems[Index++]);

		for (;;)
		{
			FindNextItem();

			if (Index == DataItems.Num())
			{
				const v_flt Extent = FMath::Max<v_flt>(Smoothness.Max, 0);
				return { BestDistance.Min - Extent, BestDistance.Max + Extent };
			}

			const TVoxelRange<v_flt> Distance = GetItemDistance(*DataItems[Index++]);
			
			if (CombineMode == EVoxelDataItemCombineMode::Min)
			{
				BestDistance = FVoxelRangeUtilities::Min(Distance, BestDistance);
			}
			else if (CombineMode == EVoxelDataItemCombineMode::Max)
			{
				BestDistance = FVoxelRangeUtilities::Max(Distance, BestDistance);
			}
			else
			{
				ensureVoxelSlow(CombineMode == EVoxelDataItemCombineMode::Sum);
				BestDistance += Distance;
			}
		}
	}
	
	/**
	 * Combine an existing generator value (must be a distance) to the data item distances
	 * See https://wiki.voxelplugin.com/Voxel_Data_Items
	 * @param	GeneratorValue	The existing value to combine with the data items
	 * @param	ItemHolder		The item holder passed to the generator
	 * @param	X				The current X coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Y				The current Y coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Z				The current Z coordinate. Should be the one passed to the generator to avoid issues with spatial hashing
	 * @param	Smoothness		The smoothness of the union/intersection. Should be >= 0
	 * @param	Mask			Use uint32(-1) to match any items
	 * @param	CombineMode		How to combine data items
	 * @return	The resulting distance
	 */
	template<bool bInvertDataItemDistances = false>
	inline v_flt CombineDataItemDistance(
		v_flt GeneratorValue,
		const FVoxelPlaceableItemHolder& ItemHolder,
		v_flt X, v_flt Y, v_flt Z,
		v_flt Smoothness,
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode)
	{
		return GetDataItemDistance<bInvertDataItemDistances>(ItemHolder, X, Y, Z, Smoothness, 0, Mask, CombineMode, &GeneratorValue);
	}
	template<bool bInvertDataItemDistances = false>
	inline TVoxelRange<v_flt> CombineDataItemDistanceRange(
		TVoxelRange<v_flt> GeneratorValue,
		const FVoxelPlaceableItemHolder& ItemHolder,
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z,
		TVoxelRange<v_flt> Smoothness,
		uint32 Mask,
		EVoxelDataItemCombineMode CombineMode)
	{
		return GetDataItemDistanceRange<bInvertDataItemDistances>(ItemHolder, X, Y, Z, Smoothness, 0, Mask, CombineMode, &GeneratorValue);
	}
};