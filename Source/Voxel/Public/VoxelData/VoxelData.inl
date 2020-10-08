// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelUtilities/VoxelOctreeUtilities.h"
#include "VoxelGenerators/VoxelGeneratorInstance.inl"

#include "Async/ParallelFor.h"

FORCEINLINE FVoxelDataOctreeBase& FVoxelData::GetOctree() const
{
	return *Octree;
}

template<typename T>
void FVoxelData::CacheBounds(const FVoxelIntBox& Bounds, bool bMultiThreaded)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	TArray<FVoxelDataOctreeLeaf*> Leaves;
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (Chunk.IsLeaf())
		{
			ensureThreadSafe(Chunk.IsLockedForWrite());

			auto& Leaf = Chunk.AsLeaf();
			auto& DataHolder = Leaf.GetData<T>();
			if (!DataHolder.HasData())
			{
				Leaves.Add(&Leaf);
			}
		}
		else
		{
			auto& Parent = Chunk.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Chunk.IsLockedForWrite());
				Parent.CreateChildren();
			}
		}
	});

	ParallelFor(Leaves.Num(), [&](int32 Index)
	{
		FVoxelDataOctreeLeaf& Leaf = *Leaves[Index];
		
		auto& DataHolder = Leaf.GetData<T>();
		DataHolder.CreateData(*this, [&](T* RESTRICT DataPtr)
		{
			TVoxelQueryZone<T> QueryZone(Leaf.GetBounds(), DataPtr);
			Leaf.GetFromGeneratorAndAssets(*Generator, QueryZone, 0);
		});

	}, !bMultiThreaded);
}

template<typename T>
void FVoxelData::ClearCacheInBounds(const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());

		auto& DataHolder = Leaf.GetData<T>();
		if (DataHolder.HasAllocation() && !DataHolder.IsDirty())
		{
			DataHolder.ClearData(*this);
		}
	});
}

template<typename T>
TArray<T> FVoxelData::Get(const FVoxelIntBox& Bounds) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	TArray<T> Result;
	Result.Empty(Bounds.Count());
	Result.SetNumUninitialized(Bounds.Count());
	TVoxelQueryZone<T> QueryZone(Bounds, Result);
	Get(QueryZone, 0);
	return Result;
}

template<typename T>
TArray<T> FVoxelData::ParallelGet(const FVoxelIntBox& Bounds, bool bForceSingleThread) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	TArray<T> Result;
	Result.SetNumUninitialized(Bounds.Count());
	TVoxelQueryZone<T> QueryZone(Bounds, Result);

	Bounds.ParallelSplit([&](const FVoxelIntBox& LocalBounds)
	{
		auto LocalQueryZone = QueryZone.ShrinkTo(LocalBounds);
		Get(LocalQueryZone, 0);
	}, bForceSingleThread);

	return Result;
}

FORCEINLINE bool FVoxelData::IsEmpty(const FVoxelIntBox& Bounds, int32 LOD) const
{
	const auto Range = GetValueRange(Bounds, LOD);
	return Range.Min.IsEmpty() == Range.Max.IsEmpty();
}

template<typename T>
FORCEINLINE T FVoxelData::GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
{
	// Clamp to world, to avoid un-editable border
	ClampToWorld(X, Y, Z);

	auto& Node = FVoxelOctreeUtilities::GetBottomNode(GetOctree(), int32(X), int32(Y), int32(Z));
	return Node.GetCustomOutput<T>(*Generator, DefaultValue, Name, X, Y, Z, LOD);
}

template<typename ... TArgs, typename F>
void FVoxelData::Set(const FVoxelIntBox& Bounds, F Apply)
{
	if (!ensure(Bounds.IsValid())) return;
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Tree)
	{
		if (Tree.IsLeaf())
		{
			auto& Leaf = Tree.AsLeaf();
			ensureThreadSafe(Leaf.IsLockedForWrite());
			FVoxelDataOctreeSetter::Set<TArgs...>(*this, Leaf, [&](auto Lambda)
			{
				Leaf.GetBounds().Overlap(Bounds).Iterate(Lambda);
			}, Apply);
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				Parent.CreateChildren();
			}
		}
	});
}

template<typename ... TArgs, typename F>
void FVoxelData::ParallelSet(const FVoxelIntBox& Bounds, F Apply, bool bForceSingleThread)
{
	if (!ensure(Bounds.IsValid())) return;

	TArray<FVoxelDataOctreeLeaf*> Leaves;
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Tree)
	{
		if (Tree.IsLeaf())
		{
			auto& Leaf = Tree.AsLeaf();
			ensureThreadSafe(Leaf.IsLockedForWrite());
			Leaves.Add(&Leaf);
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				Parent.CreateChildren();
			}
		}
	});

	ParallelFor(Leaves.Num(), [&](int32 Index)
	{
		auto& Leaf = *Leaves[Index];
		FVoxelDataOctreeSetter::Set<TArgs...>(*this, Leaf, [&](auto Lambda)
		{
			Leaf.GetBounds().Overlap(Bounds).Iterate(Lambda);
		}, Apply);
	}, bForceSingleThread);
}

template<typename T>
FORCEINLINE void FVoxelData::Set(int32 X, int32 Y, int32 Z, const T& Value)
{
	if (IsInWorld(X, Y, Z))
	{
		auto Iterate = [&](auto Lambda) { Lambda(X, Y, Z); };
		auto Apply = [&](int32, int32, int32, T& InValue) { InValue = Value; };
		auto& Leaf = *FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(GetOctree(), X, Y, Z);
		FVoxelDataOctreeSetter::Set<T>(*this, Leaf, Iterate, Apply);
	}
}

template<typename T>
FORCEINLINE T FVoxelData::Get(int32 X, int32 Y, int32 Z, int32 LOD) const
{
	// Clamp to world, to avoid un-editable border
	ClampToWorld(X, Y, Z);

	auto& Node = FVoxelOctreeUtilities::GetBottomNode(GetOctree(), int32(X), int32(Y), int32(Z));
	return Node.Get<T>(*Generator, X, Y, Z, LOD);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace FVoxelDataItemsUtilities
{
	// This will NOT add the item to the item holder
	template<typename T>
	void AddItemToLeafData(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const T& Item)
	{
	}
	// This will NOT remove the item from the item holder, but will assume it has already been removed
	template<typename T>
	void RemoveItemFromLeafData(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const T& Item)
	{
		if (!TIsSame<T, FVoxelAssetItem>::Value && !TIsSame<T, FVoxelDataItem>::Value)
		{
			return;
		}

		// Flush cache if possible
		if (!Leaf.Values.IsDirty())
		{
			Leaf.Values.ClearData(Data);
		}
		if (!Leaf.Materials.IsDirty())
		{
			Leaf.Materials.ClearData(Data);
		}

		// If something is still dirty, remove manually
		if (Leaf.Values.IsDirty())
		{
			FVoxelDataUtilities::RemoveItemFromLeafData<FVoxelValue>(Data, Leaf, Item);
		}
		if (Leaf.Materials.IsDirty())
		{
			FVoxelDataUtilities::RemoveItemFromLeafData<FVoxelMaterial>(Data, Leaf, Item);
		}
	}
}

template<>
inline void FVoxelDataItemsUtilities::AddItemToLeafData<FVoxelAssetItem>(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf,
	const FVoxelAssetItem& Item)
{
	// Flush cache if possible
	if (!Leaf.Values.IsDirty())
	{
		Leaf.Values.ClearData(Data);
	}
	if (!Leaf.Materials.IsDirty())
	{
		Leaf.Materials.ClearData(Data);
	}

	// If something is still dirty, merge manually
	if (Leaf.Values.IsDirty() || Leaf.Materials.IsDirty())
	{
		FVoxelDataUtilities::AddAssetItemToLeafData(Data, Leaf, *Item.Generator, Item.Bounds, Item.LocalToWorld, Leaf.Values.IsDirty(), Leaf.Materials.IsDirty());
	}
}

template<>
inline void FVoxelDataItemsUtilities::AddItemToLeafData<FVoxelDataItem>(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf,
	const FVoxelDataItem& Item)
{
	// Flush cache if possible
	if (!Leaf.Values.IsDirty())
	{
		Leaf.Values.ClearData(Data);
	}
	if (!Leaf.Materials.IsDirty())
	{
		Leaf.Materials.ClearData(Data);
	}

	// If something is still dirty, merge manually
	if (Leaf.Values.IsDirty())
	{
		FVoxelDataUtilities::AddItemToLeafData<FVoxelValue>(Data, Leaf, Item);
	}
	if (Leaf.Materials.IsDirty())
	{
		FVoxelDataUtilities::AddItemToLeafData<FVoxelMaterial>(Data, Leaf, Item);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T, bool bDoNotModifyExistingDataChunks, typename... TArgs>
TVoxelWeakPtr<const TVoxelDataItemWrapper<T>> FVoxelData::AddItem(TArgs&&... Args)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const auto ItemWrapper = MakeVoxelShared<TVoxelDataItemWrapper<T>>();
	ItemWrapper->Item = T{ Forward<TArgs>(Args)... };
	ItemWrapper->Data = AsShared();

	const int32 MaxPlaceableItemsPerOctree = CVarMaxPlaceableItemsPerOctree.GetValueOnAnyThread();
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), ItemWrapper->Item.Bounds, [&](FVoxelDataOctreeBase& Tree) 
	{
		if (Tree.IsLeaf())
		{
			ensureThreadSafe(Tree.IsLockedForWrite());
			
			Tree.GetItemHolder().AddItem(ItemWrapper->Item);

			if (!bDoNotModifyExistingDataChunks)
			{
				FVoxelDataItemsUtilities::AddItemToLeafData(*this, Tree.AsLeaf(), ItemWrapper->Item);
			}
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				// -1: since we're adding a new one
				if (Tree.GetItemHolder().NeedToSubdivide(MaxPlaceableItemsPerOctree - 1))
				{
					Parent.CreateChildren();
				}
				else
				{
					Tree.GetItemHolder().AddItem(ItemWrapper->Item);
				}
			}
		}
	});
	
	if (TIsSame<T, FVoxelAssetItem>::Value) { INC_DWORD_STAT(STAT_NumVoxelAssetItems); }
	if (TIsSame<T, FVoxelDisableEditsBoxItem>::Value) { INC_DWORD_STAT(STAT_NumVoxelDisableEditsItems); }
	if (TIsSame<T, FVoxelDataItem>::Value) { INC_DWORD_STAT(STAT_NumVoxelDataItems); }

	TItemData<T>& ItemsData = GetItemsData<T>();
	
	FScopeLock Lock(&ItemsData.Section);
	ItemWrapper->Index = ItemsData.Items.Add(ItemWrapper);
	return ItemWrapper;
}

template<typename T>
bool FVoxelData::RemoveItem(TVoxelWeakPtr<const TVoxelDataItemWrapper<T>>& InItem, FString& OutError)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	const auto Item = InItem.Pin();
	if (!Item.IsValid() || Item->Index == -1)
	{
		OutError = TEXT("Invalid item, or the item was already deleted");
		return false;
	}
	if (Item->Data != AsShared())
	{
		OutError = TEXT("Item doesn't belong to this data!");
		return false;
	}
	
	TItemData<T>& ItemsData = GetItemsData<T>();

	{
		FScopeLock Lock(&ItemsData.Section);
		if (!ensure(ItemsData.Items.IsValidIndex(Item->Index)))
		{
			return false;
		}
		if (!ensure(ItemsData.Items[Item->Index] == Item))
		{
			return false;
		}
	}

	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Item->Item.Bounds, [&](FVoxelDataOctreeBase& Tree)
	{
		if (Tree.IsLeafOrHasNoChildren())
		{
			ensureThreadSafe(Tree.IsLockedForWrite());

			Tree.GetItemHolder().RemoveItem(Item->Item);

			if (Tree.IsLeaf())
			{
				FVoxelDataItemsUtilities::RemoveItemFromLeafData(*this, Tree.AsLeaf(), Item->Item);
			}
		}
	});
	
	if (TIsSame<T, FVoxelAssetItem>::Value) { DEC_DWORD_STAT(STAT_NumVoxelAssetItems); }
	if (TIsSame<T, FVoxelDisableEditsBoxItem>::Value) { DEC_DWORD_STAT(STAT_NumVoxelDisableEditsItems); }
	if (TIsSame<T, FVoxelDataItem>::Value) { DEC_DWORD_STAT(STAT_NumVoxelDataItems); }
	
	FScopeLock Lock(&ItemsData.Section);
	// Make sure our item is the last one
	ItemsData.Items.Swap(Item->Index, ItemsData.Items.Num() - 1);
	// Fixup the one we swapped (could be us, but that's fine)
	ItemsData.Items[Item->Index]->Index = Item->Index;
	// Pop the item
	ensure(ItemsData.Items.Pop(false) == Item);
	// Clear the index, in case we try to remove the item twice
	Item->Index = -1;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
inline FVoxelData::TItemData<FVoxelAssetItem>& FVoxelData::GetItemsData<FVoxelAssetItem>()
{
	return AssetItemsData;
}

template<>
inline FVoxelData::TItemData<FVoxelDisableEditsBoxItem>& FVoxelData::GetItemsData<FVoxelDisableEditsBoxItem>()
{
	return DisableEditsItemsData;
}

template<>
inline FVoxelData::TItemData<FVoxelDataItem>& FVoxelData::GetItemsData<FVoxelDataItem>()
{
	return DataItemsData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T, typename TApplyOld, typename TApplyNew>
void FVoxelDataUtilities::MigrateLeafDataToNewGenerator(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf,
	const FVoxelIntBox& BoundsToMigrate,
	TApplyOld ApplyOldGenerator,
	TApplyNew ApplyNewGenerator)
{
	TVoxelDataOctreeLeafData<T>& DataHolder = Leaf.GetData<T>();
	if (!ensure(DataHolder.IsDirty()))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = Leaf.GetBounds().Overlap(BoundsToMigrate);
	const FIntVector Size = Bounds.Size();

	// Revert to the old generator to query the old data
	ApplyOldGenerator();
	
	TArray<T> OldGeneratorData;
	OldGeneratorData.SetNumUninitialized(Bounds.Count());
	{
		TVoxelQueryZone<T> QueryZone(Bounds, OldGeneratorData.GetData());
		Leaf.GetFromGeneratorAndAssets<T>(*Data.Generator, QueryZone, 0);
	}

	// Switch back to the new generator, and query the new data
	ApplyNewGenerator();
	
	TArray<T> NewGeneratorData;
	NewGeneratorData.SetNumUninitialized(Bounds.Count());
	{
		TVoxelQueryZone<T> QueryZone(Bounds, NewGeneratorData.GetData());
		Leaf.GetFromGeneratorAndAssets<T>(*Data.Generator, QueryZone, 0);
	}

	// Update all the data that was the same as the old generator to the new generator
	FVoxelDataOctreeSetter::Set<T>(Data, Leaf, [&](auto Lambda) { Bounds.Iterate(Lambda); }, 
		[&](int32 X, int32 Y, int32 Z, T& Value)
		{
			const int32 Index = FVoxelUtilities::Get3DIndex(Size, X, Y, Z, Bounds.Min);

			if (Value == FVoxelUtilities::Get(OldGeneratorData, Index))
			{
				// Switch the edited value to the new value if it wasn't edited
				Value = FVoxelUtilities::Get(NewGeneratorData, Index);
			}
		});
}

template<typename T, typename TItem>
void FVoxelDataUtilities::AddItemToLeafData(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf,
	const TItem& Item)
{
	MigrateLeafDataToNewGenerator<T>(
		Data,
		Leaf,
		Item.Bounds,
		[&]() { ensure(Leaf.GetItemHolder().RemoveItem(Item)); },
		[&]() { Leaf.GetItemHolder().AddItem(Item); });
}

template<typename T, typename TItem>
void FVoxelDataUtilities::RemoveItemFromLeafData(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf,
	const TItem& Item)
{
	ensureVoxelSlowNoSideEffects(!Leaf.GetItemHolder().RemoveItem(Item));
	MigrateLeafDataToNewGenerator<T>(
		Data,
		Leaf,
		Item.Bounds,
		[&]() { Leaf.GetItemHolder().AddItem(Item); },
		[&]() { ensure(Leaf.GetItemHolder().RemoveItem(Item)); });
}