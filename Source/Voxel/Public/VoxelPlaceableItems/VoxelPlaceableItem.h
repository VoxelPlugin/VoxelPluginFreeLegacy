// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "Containers/ArrayView.h"

class VOXEL_API FVoxelPlaceableItem
{
public:
	const FIntBox Bounds;
	const int Priority;

	FVoxelPlaceableItem(const FIntBox& InBounds, int Priority = 0) : Bounds(InBounds), Priority(Priority) {}

	enum Ids : uint8
	{
		EVoxelPerlinWorm       = 0,
		EVoxelAssetItem        = 1,
		EVoxelExclusionBoxItem = 2,
		EVoxelDisableEditsBox  = 3,
		EBuiltinMax			   = 32,
	};

	inline bool operator<(const FVoxelPlaceableItem& Other) const
	{
		return Priority > Other.Priority;
	}
};

struct VOXEL_API FVoxelPlaceableItemHolder
{
public:
	template<typename T>
	void AddItem(T* Item)
	{
		AddItem(T::StaticId(), static_cast<FVoxelPlaceableItem*>(Item));
	}
	void AddItem(uint8 Id, FVoxelPlaceableItem* Item)
	{
		while (Items.Num() <= (int)Id)
		{
			Items.Emplace();
		}

		Items[Id].Add(Item);
		Items[Id].Sort();
	}

	template<typename T>
	const TArrayView<T* const> GetItems() const
	{
		if (Items.IsValidIndex(T::StaticId()))
		{
			auto& ItemsT = Items[T::StaticId()];
			return TArrayView<T* const>((T* const *)ItemsT.GetData(), ItemsT.Num());
		}
		else
		{
			return TArrayView<T*>();
		}
	}
	const TArrayView<FVoxelPlaceableItem* const> GetItems(uint8 Id) const
	{
		if (Items.IsValidIndex(Id))
		{
			auto& ItemsT = Items[Id];
			return TArrayView<FVoxelPlaceableItem* const>(ItemsT.GetData(), ItemsT.Num());
		}
		else
		{
			return TArrayView<FVoxelPlaceableItem*>();
		}
	}

	const TArray<TArray<FVoxelPlaceableItem*>>& GetAllItems() const { return Items; }
	
	inline int Num() const
	{
		int N = 0;
		for (auto& ItemsArray : Items)
		{
			N += ItemsArray.Num();
		}
		return N;
	}

	inline bool IsEmpty() const
	{
		return Items.Num() == 0;
	}

private:
	TArray<TArray<FVoxelPlaceableItem*>> Items;
};
