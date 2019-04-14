// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "Containers/ArrayView.h"

class FVoxelDataOctree;

class VOXEL_API FVoxelPlaceableItem
{
public:
	const uint8 ItemId; // Item class id
	const FIntBox Bounds;
	const int32 Priority;
	int32 ItemIndex = -1;

	FVoxelPlaceableItem(uint8 ItemId, const FIntBox& InBounds, int32 Priority = 0)
		: ItemId(ItemId)
		, Bounds(InBounds)
		, Priority(Priority)
	{
	}
	virtual ~FVoxelPlaceableItem() = default;

	inline bool operator<(const FVoxelPlaceableItem& Other) const
	{
		return Priority > Other.Priority;
	}

public:
	virtual FString GetDescription() const = 0;
	virtual void Save(FArchive& Ar) const = 0;
	virtual TSharedRef<FVoxelPlaceableItem> Load(FArchive& Ar) const = 0;
	virtual bool ShouldBeSaved() const { return true; }
	virtual void MergeWithOctree(FVoxelDataOctree* Octree) const = 0; // Octree is a LOD 0 created octree
	
public:
	inline static FVoxelPlaceableItem* GetDefault(uint8 ItemId)
	{
		checkf(DefaultItems.IsValidIndex(ItemId) && DefaultItems[ItemId], TEXT("Item with Id=%d isn't constructed. You need to create a FVoxelPlaceableItem::TDefaultConstructor<YourItem>(new YourItem(your args));"), ItemId);
		return DefaultItems[ItemId];
	}

	template<typename T>
	class TDefaultConstructor
	{
	public:
		TDefaultConstructor(T* DefaultItem)
		{
			if (FVoxelPlaceableItem::DefaultItems.Num() <= T::StaticId())
			{
				FVoxelPlaceableItem::DefaultItems.SetNum(T::StaticId() + 1);
			}
			auto*& Element = FVoxelPlaceableItem::DefaultItems[T::StaticId()];
			ensure(!Element);
			Element = DefaultItem;
		}
	};

private:
	static TArray<FVoxelPlaceableItem*> DefaultItems;

	template<typename T>
	friend class TDefaultConstructor;
};

inline FArchive& operator<<(FArchive& Ar, TSharedPtr<FVoxelPlaceableItem>& Item)
{
	uint8 ItemId = Ar.IsSaving() ? Item->ItemId : 0;
	Ar << ItemId;
	check(ItemId != 0);

	if (Ar.IsLoading())
	{
		Item = FVoxelPlaceableItem::GetDefault(ItemId)->Load(Ar);
	}
	else
	{
		Item->Save(Ar);
	}
	return Ar;
}

namespace EVoxelPlaceableItemId
{
	enum Enum : uint8
	{
		Invalid         = 0,
		PerlinWorm      = 1,
		Asset           = 2,
		ExclusionBox    = 3,
		DisableEditsBox = 4,
		BuiltinMax      = 32,
	};
}

struct VOXEL_API FVoxelPlaceableItemHolder
{
public:
	inline void AddItem(FVoxelPlaceableItem* Item)
	{
		int32 ItemId = Item->ItemId;
		if (Items.Num() <= ItemId)
		{
			Items.SetNum(ItemId + 1);
			Items.Shrink();
		}
		Items[ItemId].Add(Item);
		Items[ItemId].Sort();
		Items[ItemId].Shrink();
	}

	inline void RemoveItem(FVoxelPlaceableItem* Item)
	{
		int32 ItemId = Item->ItemId;
		if (Items.IsValidIndex(ItemId))
		{
			Items[ItemId].Remove(Item);
		}
	}

	inline const TArray<FVoxelPlaceableItem*>& GetItems(uint8 ItemId) const
	{
		if (Items.IsValidIndex(ItemId))
		{
			return Items[ItemId];
		}
		else
		{
			static TArray<FVoxelPlaceableItem*> EmptyArray;
			return EmptyArray;
		}
	}
	template<typename T>
	inline const TArray<T*>& GetItems() const
	{
		return reinterpret_cast<const TArray<T*>&>(GetItems(T::StaticId()));
	}

	inline const TArray<TArray<FVoxelPlaceableItem*>>& GetAllItems() const
	{
		return Items;
	}
	
	inline int32 Num() const
	{
		int32 N = 0;
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
