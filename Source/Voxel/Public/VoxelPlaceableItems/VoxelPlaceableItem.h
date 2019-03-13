// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "Containers/ArrayView.h"

class VOXEL_API FVoxelPlaceableItem
{
public:
	const uint64 UniqueId;
	const FIntBox Bounds;
	const int Priority;

	FVoxelPlaceableItem(const FIntBox& InBounds, int Priority = 0) : UniqueId(StaticUniqueId++), Bounds(InBounds), Priority(Priority) {}

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

private:
	static uint64 StaticUniqueId;
};

struct VOXEL_API FVoxelPlaceableItemHolder
{
public:
	void AddItem(uint8 Id, FVoxelPlaceableItem* Item)
	{
		if (Items.Num() <= (int)Id)
		{
			Items.SetNum(Id + 1);
			Items.Shrink();
		}
		Items[Id].Add(Item);
		Items[Id].Shrink();
	}
	template<typename T>
	void AddItem(T* Item)
	{
		AddItem(T::StaticId(), static_cast<FVoxelPlaceableItem*>(Item));
	}

	const TArray<FVoxelPlaceableItem*>& GetItems(uint8 Id) const
	{
		if (Items.IsValidIndex(Id))
		{
			return Items[Id];
		}
		else
		{
			static TArray<FVoxelPlaceableItem*> EmptyArray;
			return EmptyArray;
		}
	}
	template<typename T>
	const TArray<T*>& GetItems() const
	{
		return reinterpret_cast<const TArray<T*>&>(GetItems(T::StaticId()));
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
