// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "NodeFunctions/VoxelNodeFunctions.h"
#include "VoxelUtilities/VoxelSDFUtilities.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.inl"

namespace FVoxelNodeFunctions
{
	inline v_flt GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, v_flt X, v_flt Y, v_flt Z, v_flt Smoothness, v_flt Default)
	{
		auto& DataItems = ItemHolder.GetDataItems();
		if (DataItems.Num() == 0)
		{
			return Default;
		}

		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			return Item.WorldGenerator->GetValue(X, Y, Z, 0, Stack);
		};

		v_flt BestDistance = GetItemDistance(*DataItems[0]);
		for (int32 Index = 1; Index < DataItems.Num(); Index++)
		{
			const v_flt Distance = GetItemDistance(*DataItems[Index]);
			BestDistance = Smoothness <= 0 ? FMath::Min(Distance, BestDistance) : FVoxelSDFUtilities::opSmoothUnion(Distance, BestDistance, Smoothness);
		}
		return BestDistance;
	}

	inline TVoxelRange<v_flt> GetDataItemDistance(const FVoxelPlaceableItemHolder& ItemHolder, TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z, TVoxelRange<v_flt> Smoothness, TVoxelRange<v_flt> Default)
	{
		auto& DataItems = ItemHolder.GetDataItems();
		if (DataItems.Num() == 0)
		{
			return Default;
		}

		const FVoxelIntBox Bounds = FVoxelNodeFunctions::BoundsFromRanges(X, Y, Z);

		const auto GetItemDistance = [&](const FVoxelDataItem& Item)
		{
			const auto Stack = FVoxelItemStack::Empty.WithCustomData(&Item.Data);
			return Item.WorldGenerator->GetValueRange(Bounds, 0, Stack);
		};

		TVoxelRange<v_flt> BestDistance = GetItemDistance(*DataItems[0]);
		for (int32 Index = 1; Index < DataItems.Num(); Index++)
		{
			const TVoxelRange<v_flt> Distance = GetItemDistance(*DataItems[Index]);
			BestDistance = FVoxelRangeUtilities::Min(Distance, BestDistance);
		}
		const v_flt Extent = FMath::Max<v_flt>(Smoothness.Max, 0);
		return { BestDistance.Min - Extent, BestDistance.Max + Extent };
	}
}