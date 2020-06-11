// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelValue.h"
#include "VoxelMinimal.h"

struct FVoxelMaterial;
class FVoxelPlaceableItemHolder;
class AVoxelWorld;

class VOXEL_API FVoxelPlaceableItem
{
public:
	const uint8 ItemId; // Item class id
	const FVoxelIntBox Bounds;
	const int32 Priority;
	int32 ItemIndex = -1; // Index in the VoxelData array

	FVoxelPlaceableItem(uint8 ItemId, const FVoxelIntBox& InBounds, int32 Priority)
		: ItemId(ItemId)
		, Bounds(InBounds)
		, Priority(Priority)
	{
	}
	virtual ~FVoxelPlaceableItem() = default;

	inline bool operator<(const FVoxelPlaceableItem& Other) const
	{
		return Priority < Other.Priority;
	}

	template<typename T>
	inline bool IsA() const
	{
		return ItemId == T::StaticId();
	}

public:
	virtual FString GetDescription() const = 0;
	virtual void Save(FArchive& Ar) const = 0;
	virtual bool ShouldBeSaved() const { return true; }
};

struct VOXEL_API FVoxelPlaceableItemLoader
{
	explicit FVoxelPlaceableItemLoader(uint8 ItemId);
	virtual ~FVoxelPlaceableItemLoader() = default;

	virtual TVoxelSharedRef<FVoxelPlaceableItem> Load(FArchive& Ar, const AVoxelWorld* VoxelWorld) const = 0;

public:
	static FVoxelPlaceableItemLoader* GetLoader(uint8 ItemId);

private:
	static TArray<FVoxelPlaceableItemLoader*>& GetStaticLoaders();
};

VOXEL_API FArchive& SerializeVoxelItem(FArchive& Ar, const AVoxelWorld* VoxelWorld, TVoxelSharedPtr<FVoxelPlaceableItem>& Item);

namespace EVoxelPlaceableItemId
{
	enum Enum : uint8
	{
		Invalid         = 0,
		PerlinWorm      = 1,
		Asset           = 2,
		DisableEditsBox = 3,
		BuiltinMax      = 32,
	};
}

class VOXEL_API FVoxelPlaceableItemHolder
{
public:
	static const FVoxelPlaceableItemHolder Empty;

	inline void AddItem(FVoxelPlaceableItem* Item)
	{
		const int32 ItemId = Item->ItemId;
		if (Items.Num() <= ItemId)
		{
			Items.SetNum(ItemId + 1);
			Items.Shrink();
		}
		auto& ItemArray = Items[ItemId];
		ensure(!ItemArray.Contains(Item));
		ItemArray.Add(Item);
		ItemArray.Sort();
		ItemArray.Shrink();
	}

	inline void RemoveItem(FVoxelPlaceableItem* Item)
	{
		const int32 ItemId = Item->ItemId;
		if (Items.IsValidIndex(ItemId))
		{
			Items[ItemId].Remove(Item);
		}
	}

	FORCEINLINE TArrayView<FVoxelPlaceableItem* const> GetItems(uint8 ItemId) const
	{
		if (Items.IsValidIndex(ItemId))
		{
			return Items.GetData()[ItemId];
		}
		else
		{
			return {};
		}
	}
	template<typename T>
	FORCEINLINE TArrayView<T* const> GetItems() const
	{
		const auto ItemId = T::StaticId();
		if (Items.IsValidIndex(ItemId))
		{
			const auto& Array = Items.GetData()[ItemId];
			return TArrayView<T* const>(reinterpret_cast<T* const *>(Array.GetData()), Array.Num());
		}
		else
		{
			return {};
		}
	}

	inline const TArray<TArray<FVoxelPlaceableItem*>>& GetAllItems() const
	{
		return Items;
	}

	inline int32 Num(uint8 ItemId) const
	{
		return Items.IsValidIndex(ItemId) ? Items[ItemId].Num() : 0;
	}

	inline bool IsEmpty() const
	{
		return Items.Num() == 0;
	}

private:
	TArray<TArray<FVoxelPlaceableItem*>> Items;
};