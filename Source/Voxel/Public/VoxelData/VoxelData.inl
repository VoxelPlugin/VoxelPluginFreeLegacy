// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelUtilities/VoxelOctreeUtilities.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.inl"

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
			Leaf.GetFromGeneratorAndAssets(*WorldGenerator, QueryZone, 0);
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
	return Node.GetCustomOutput<T>(*WorldGenerator, DefaultValue, Name, X, Y, Z, LOD);
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
	return Node.Get<T>(*WorldGenerator, X, Y, Z, LOD);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace FVoxelDataItemsUtilities
{
	template<typename T>
	void AddItemToDirtyLeaf(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const T& Item)
	{
		check(false);
	}
}

template<>
inline void FVoxelDataItemsUtilities::AddItemToDirtyLeaf<FVoxelAssetItem>(const FVoxelData& Data, FVoxelDataOctreeLeaf& Leaf, const FVoxelAssetItem& Item)
{
	FVoxelDataUtilities::AddAssetItemDataToLeaf(Data, Leaf, *Item.WorldGenerator, Item.LocalToWorld, Leaf.Values.IsDirty(), Leaf.Materials.IsDirty());
}

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

			if (!bDoNotModifyExistingDataChunks && TIsSame<T, FVoxelAssetItem>::Value)
			{
				auto& Leaf = Tree.AsLeaf();

				// Flush cache if possible
				if (!Leaf.Values.IsDirty())
				{
					Leaf.Values.ClearData(*this);
				}
				if (!Leaf.Materials.IsDirty())
				{
					Leaf.Materials.ClearData(*this);
				}

				// If something is still dirty, merge manually
				if (Leaf.Values.IsDirty() || Leaf.Materials.IsDirty())
				{
					FVoxelDataItemsUtilities::AddItemToDirtyLeaf(*this, Leaf, ItemWrapper->Item);
				}
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

			if (Tree.IsLeaf() && TIsSame<T, FVoxelAssetItem>::Value)
			{
				auto& Leaf = Tree.AsLeaf();

				// Flush cache if possible to clear cached item data
				if (!Leaf.Values.IsDirty())
				{
					Leaf.Values.ClearData(*this);
				}
				if (!Leaf.Materials.IsDirty())
				{
					Leaf.Materials.ClearData(*this);
				}
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