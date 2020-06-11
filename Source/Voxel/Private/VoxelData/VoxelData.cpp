// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelWorld.h"

#include "Misc/ScopeLock.h"
#include "Async/Async.h"

VOXEL_API TAutoConsoleVariable<int32> CVarMaxPlaceableItemsPerOctree(
		TEXT("voxel.data.MaxPlaceableItemsPerOctree"),
		32,
		TEXT("Max number of placeable items per data octree node. If more placeable items are added, the node is split"),
		ECVF_Default);

VOXEL_API TAutoConsoleVariable<int32> CVarStoreSpecialValueForGeneratorValuesInSaves(
		TEXT("voxel.data.StoreSpecialValueForGeneratorValuesInSaves"),
		1,
		TEXT("If true, will store FVoxelValue::Special() instead of the value if it's equal to the generator value when saving. Reduces save size a lot, but increases save time a lot too.\n")
		TEXT("Important: must be the same when saving & loading!"),
		ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline auto CreateWorldGenerator(const AVoxelWorld* World)
{
	auto WorldGeneratorInstance = World->WorldGenerator.GetInstance(true);
	WorldGeneratorInstance->Init(World->GetInitStruct());
	return WorldGeneratorInstance;
}

inline int32 ClampDataDepth(int32 Depth)
{
	return FMath::Max(1, FVoxelUtilities::ClampDepth<DATA_CHUNK_SIZE>(Depth));
}

FVoxelDataSettings::FVoxelDataSettings(const AVoxelWorld* World, EVoxelPlayType PlayType)
	: Depth(ClampDataDepth(FVoxelUtilities::ConvertDepth<RENDER_CHUNK_SIZE, DATA_CHUNK_SIZE>(World->RenderOctreeDepth)))
	, WorldBounds(World->GetWorldBounds())
	, WorldGenerator(CreateWorldGenerator(World))
	, bEnableMultiplayer(false)
	, bEnableUndoRedo(PlayType == EVoxelPlayType::Game ? World->bEnableUndoRedo : true)
{
}

FVoxelDataSettings::FVoxelDataSettings(
	int32 Depth, 
	const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& WorldGenerator,
	bool bEnableMultiplayer,
	bool bEnableUndoRedo)
	: Depth(ClampDataDepth(Depth))
	, WorldBounds(FVoxelUtilities::GetBoundsFromDepth<DATA_CHUNK_SIZE>(Depth))
	, WorldGenerator(WorldGenerator)
	, bEnableMultiplayer(bEnableMultiplayer)
	, bEnableUndoRedo(bEnableUndoRedo)
{

}

FVoxelDataSettings::FVoxelDataSettings(
	const FVoxelIntBox& WorldBounds, 
	const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& WorldGenerator, 
	bool bEnableMultiplayer, 
	bool bEnableUndoRedo)
	: Depth(ClampDataDepth(FVoxelUtilities::GetOctreeDepthContainingBounds<DATA_CHUNK_SIZE>(WorldBounds)))
	, WorldBounds(WorldBounds)
	, WorldGenerator(WorldGenerator)
	, bEnableMultiplayer(bEnableMultiplayer)
	, bEnableUndoRedo(bEnableUndoRedo)
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelData::FVoxelData(const FVoxelDataSettings& Settings)
	: IVoxelData(Settings.Depth, Settings.WorldBounds, Settings.bEnableMultiplayer, Settings.bEnableUndoRedo, Settings.WorldGenerator)
	, Octree(MakeUnique<FVoxelDataOctreeParent>(Depth))
{
	check(Depth > 0);
	check(Octree->GetBounds().Contains(WorldBounds));
}

TVoxelSharedRef<FVoxelData> FVoxelData::Create(const FVoxelDataSettings& Settings, int32 DataOctreeInitialSubdivisionDepth)
{
	auto* Data = new FVoxelData(Settings);

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Subdivide Data");
		// Subdivide tree a few levels on start to avoid having update tasks locking the entire octree
		FVoxelOctreeUtilities::IterateEntireTree(Data->GetOctree(), [&](FVoxelDataOctreeBase& Chunk)
		{
			if (!Chunk.IsLeaf() && Settings.Depth - Chunk.Height < DataOctreeInitialSubdivisionDepth)
			{
				Chunk.AsParent().CreateChildren();
			}
		});
	}
	return MakeShareable(Data);
}

FVoxelData::~FVoxelData()
{
	ClearData();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelDataOctreeLocker
{
public:
	const EVoxelLockType LockType;
	const FVoxelIntBox Bounds;
	const FName Name;

	FVoxelDataOctreeLocker(EVoxelLockType LockType, const FVoxelIntBox& Bounds, FName Name)
		: LockType(LockType)
		, Bounds(Bounds)
		, Name(Name)
	{
	}

	TArray<FVoxelOctreeId> Lock(FVoxelDataOctreeBase& Octree)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		if (!Octree.GetBounds().Intersect(Bounds))
		{
			return {};
		}
		
		LockImpl(Octree);

#if VOXEL_DEBUG
		FVoxelOctreeUtilities::IterateTreeInBounds(Octree, Bounds, [&](FVoxelDataOctreeBase& Tree)
		{
			if (Tree.IsLeafOrHasNoChildren())
			{
				if (LockType == EVoxelLockType::Read)
				{
					ensureThreadSafe(Tree.IsLockedForRead());
				}
				else
				{
					ensureThreadSafe(Tree.IsLockedForWrite());
				}
			}
		});
#endif
		
		return MoveTemp(LockedOctrees);
	}

private:
	TArray<FVoxelOctreeId> LockedOctrees;

	void LockImpl(FVoxelDataOctreeBase& Octree)
	{
		checkVoxelSlow(Bounds.Intersect(Octree.GetBounds()));

		Octree.Mutex.Lock(LockType);

		// Need to be locked to check IsLeafOrHasNoChildren
		if (Octree.IsLeafOrHasNoChildren())
		{
			LockedOctrees.Add(Octree.GetId());
		}
		else
		{
			Octree.Mutex.Unlock(LockType);

			auto& Parent = Octree.AsParent();
			for (auto& Child : Parent.GetChildren())
			{
				if (Child.GetBounds().Intersect(Bounds))
				{
					LockImpl(Child);
				}
			}
		}
	}
};
class FVoxelDataOctreeUnlocker
{
public:
	const EVoxelLockType LockType;
	const TArray<FVoxelOctreeId>& LockedOctrees;

	FVoxelDataOctreeUnlocker(EVoxelLockType LockType, const TArray<FVoxelOctreeId>& LockedOctrees)
		: LockType(LockType)
		, LockedOctrees(LockedOctrees)
	{
	}

	void Unlock(FVoxelDataOctreeBase& Octree)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		UnlockImpl(Octree);
		check(LockedOctreesIndex == LockedOctrees.Num());
	}
	
private:
	int32 LockedOctreesIndex = 0;
	
	void UnlockImpl(FVoxelDataOctreeBase& Octree)
	{
		if (LockedOctreesIndex == LockedOctrees.Num())
		{
			return;
		}

		if (LockedOctrees[LockedOctreesIndex] == Octree.GetId())
		{
			LockedOctreesIndex++;
			
			ensure(
				!LockedOctrees.IsValidIndex(LockedOctreesIndex) ||
				!Octree.IsInOctree(LockedOctrees[LockedOctreesIndex].Position));

			Octree.Mutex.Unlock(LockType);
		}
		else if (Octree.IsInOctree(LockedOctrees[LockedOctreesIndex].Position))
		{
			checkVoxelSlow(LockedOctrees[LockedOctreesIndex].Height < Octree.Height);
			checkVoxelSlow(!Octree.IsLeaf());
			auto& Parent = Octree.AsParent();
			for (auto& Child : Parent.GetChildren())
			{
				UnlockImpl(Child);
			}
		}
	}
};

TUniquePtr<FVoxelDataLockInfo> FVoxelData::Lock(EVoxelLockType LockType, const FVoxelIntBox& Bounds, FName Name) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	ensure(Bounds.IsValid());

	MainLock.Lock(EVoxelLockType::Read);

	auto LockInfo = TUniquePtr<FVoxelDataLockInfo>(new FVoxelDataLockInfo());
	LockInfo->Name = Name;
	LockInfo->LockType = LockType;
	LockInfo->LockedOctrees = FVoxelDataOctreeLocker(LockType, Bounds, Name).Lock(GetOctree());
	return LockInfo;
}

void FVoxelData::Unlock(TUniquePtr<FVoxelDataLockInfo> LockInfo) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	check(LockInfo.IsValid());

	FVoxelDataOctreeUnlocker(LockInfo->LockType, LockInfo->LockedOctrees).Unlock(GetOctree());
	
	MainLock.Unlock(EVoxelLockType::Read);

	LockInfo->LockedOctrees.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::ClearData()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	MainLock.Lock(EVoxelLockType::Write);
	{
		// Clear the data to have clean memory reports
		FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			Leaf.GetData<FVoxelValue>().ClearData(*this);
			Leaf.GetData<FVoxelMaterial>().ClearData(*this);
			Leaf.GetData<FVoxelFoliage>().ClearData(*this);
		});

		ensure(GetDirtyMemory().Values.GetValue() == 0);
		ensure(GetDirtyMemory().Materials.GetValue() == 0);
		ensure(GetDirtyMemory().Foliage.GetValue() == 0);

		ensure(GetCachedMemory().Values.GetValue() == 0);
		ensure(GetCachedMemory().Materials.GetValue() == 0);
		ensure(GetCachedMemory().Foliage.GetValue() == 0);

		Octree = MakeUnique<FVoxelDataOctreeParent>(Depth);
	}
	MainLock.Unlock(EVoxelLockType::Write);

	HistoryPosition = 0;
	MaxHistoryPosition = 0;
	UndoFramesBounds.Reset();
	RedoFramesBounds.Reset();
	MarkAsDirty();
	
	FScopeLock Lock(&ItemsSection);
	FreeItems.Empty();
	Items.Empty();
	ItemFrame = MakeUnique<FItemFrame>();
	ItemUndoFrames.Empty();
	ItemRedoFrames.Empty();
	LeavesWithRedoStackStack.Empty();
}

void FVoxelData::ClearOctreeData(TArray<FVoxelIntBox>& OutBoundsToUpdate)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());
		bool bUpdate = false;
		if (Leaf.GetData<FVoxelValue>().IsDirty())
		{
			bUpdate = true;
			Leaf.GetData<FVoxelValue>().ClearData(*this);
		}
		if (Leaf.GetData<FVoxelMaterial>().IsDirty())
		{
			bUpdate = true;
			Leaf.GetData<FVoxelMaterial>().ClearData(*this);
		}
		if (Leaf.GetData<FVoxelFoliage>().IsDirty())
		{
			bUpdate = true;
			Leaf.GetData<FVoxelFoliage>().ClearData(*this);
		}
		if (bUpdate)
		{
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});
}

template<typename T>
void FVoxelData::CacheBounds(const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (Chunk.IsLeaf())
		{
			ensureThreadSafe(Chunk.IsLockedForWrite());

			auto& Leaf = Chunk.AsLeaf();
			auto& DataHolder = Leaf.GetData<T>();
			if (!DataHolder.GetDataPtr() && !DataHolder.IsSingleValue())
			{
				DataHolder.CreateDataPtr(*this);
				TVoxelQueryZone<T> QueryZone(Chunk.GetBounds(), DataHolder.GetDataPtr());
				Leaf.GetFromGeneratorAndAssets(*WorldGenerator, QueryZone, 0);
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
}

template VOXEL_API void FVoxelData::CacheBounds<FVoxelValue   >(const FVoxelIntBox&);
template VOXEL_API void FVoxelData::CacheBounds<FVoxelMaterial>(const FVoxelIntBox&);

template<typename T>
void FVoxelData::ClearCacheInBounds(const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());

		auto& DataHolder = Leaf.GetData<T>();
		if (DataHolder.GetDataPtr() && !DataHolder.IsDirty())
		{
			DataHolder.ClearData(*this);
		}
	});
}

template VOXEL_API void FVoxelData::ClearCacheInBounds<FVoxelValue   >(const FVoxelIntBox&);
template VOXEL_API void FVoxelData::ClearCacheInBounds<FVoxelMaterial>(const FVoxelIntBox&);

template<typename T>
void FVoxelData::CheckIsSingle(const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());

		if (!Leaf.GetData<T>().IsSingleValue())
		{
			Leaf.GetData<T>().TryCompressToSingleValue(*this);
		}
	});
}

template VOXEL_API void FVoxelData::CheckIsSingle<FVoxelValue   >(const FVoxelIntBox&);
template VOXEL_API void FVoxelData::CheckIsSingle<FVoxelMaterial>(const FVoxelIntBox&);

template<typename T>
void FVoxelData::Get(TVoxelQueryZone<T>& GlobalQueryZone, int32 LOD) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	// TODO this is very inefficient for high LODs as we don't early exit when we already know we won't be reading any data in the chunk
	// TODO BUG: this is also querying data multiple times if we have edited data!
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), GlobalQueryZone.Bounds, [&](FVoxelDataOctreeBase& InOctree)
	{
		if (!InOctree.IsLeafOrHasNoChildren()) return;
		ensureThreadSafe(InOctree.IsLockedForRead());
		
		auto QueryZone = GlobalQueryZone.ShrinkTo(InOctree.GetBounds());

		if (InOctree.IsLeaf())
		{
			auto& Data = InOctree.AsLeaf().GetData<T>();

			if (Data.GetDataPtr())
			{
				VOXEL_SLOW_SCOPE_COUNTER("Copy Data");
				const FIntVector Min = InOctree.GetMin();
				T* RESTRICT DataPtr = Data.GetDataPtr();
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
				{
					for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
					{
						for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
						{
							const int32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);
							QueryZone.Set(X, Y, Z, DataPtr[Index]);
						}
					}
				}
				return;
			}
			if (Data.IsSingleValue())
			{
				VOXEL_SLOW_SCOPE_COUNTER("Copy Single Value");
				const T SingleValue = Data.GetSingleValue();
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
				{
					for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
					{
						for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
						{
							QueryZone.Set(X, Y, Z, SingleValue);
						}
					}
				}
				return;
			}
		}
		
		InOctree.GetFromGeneratorAndAssets<T>(*WorldGenerator, QueryZone, LOD);
	});

	// Handle data outside of the world bounds
	// Can happen on edges with marching cubes, as it's querying N + 1 voxels with N a power of 2
	// Note that we should probably use WorldBounds here, but doing so with a correct handling of Step is quite complex
	const FVoxelIntBox OctreeBounds = Octree->GetBounds();
	check(OctreeBounds.IsMultipleOf(GlobalQueryZone.Step));
	if (!OctreeBounds.Contains(GlobalQueryZone.Bounds))
	{
		for (auto& LocalBounds : GlobalQueryZone.Bounds.Difference(OctreeBounds))
		{
			check(LocalBounds.IsMultipleOf(GlobalQueryZone.Step));
			auto LocalQueryZone = GlobalQueryZone.ShrinkTo(LocalBounds);
			for (VOXEL_QUERY_ZONE_ITERATE(LocalQueryZone, X))
			{
				for (VOXEL_QUERY_ZONE_ITERATE(LocalQueryZone, Y))
				{
					for (VOXEL_QUERY_ZONE_ITERATE(LocalQueryZone, Z))
					{
						// Get will handle clamping to the world bounds
						LocalQueryZone.Set(X, Y, Z, Get<T>(X, Y, Z, LOD));
					}
				}
			}
		}
	}
}

template VOXEL_API void FVoxelData::Get<FVoxelValue   >(TVoxelQueryZone<FVoxelValue   >&, int32) const;
template VOXEL_API void FVoxelData::Get<FVoxelMaterial>(TVoxelQueryZone<FVoxelMaterial>&, int32) const;

TVoxelRange<FVoxelValue> FVoxelData::GetValueRange(const FVoxelIntBox& InBounds, int32 LOD) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	ensure(InBounds.IsValid());
	
	const auto Apply = [&](FVoxelDataOctreeBase& Tree)
	{
		const auto TreeBounds = Tree.GetBounds();
		ensureVoxelSlowNoSideEffects(InBounds.Intersect(TreeBounds));
		
		const auto QueryBounds = InBounds.Overlap(TreeBounds);
		ensureVoxelSlowNoSideEffects(QueryBounds.IsValid());
		
		if (Tree.IsLeaf())
		{
			auto& Data = Tree.AsLeaf().GetData<FVoxelValue>();
			if (Data.IsSingleValue())
			{
				return TVoxelRange<FVoxelValue>(Data.GetSingleValue());
			}
			if (Data.IsDirty())
			{
				// Could also store the data bounds, but that would require to track it when editing. Probably not worth the added cost.
				return TVoxelRange<FVoxelValue>::Infinite();
			}
		}

		auto& ItemHolder = Tree.GetItemHolder();
		const auto Assets = ItemHolder.GetItems<FVoxelAssetItem>();

		TOptional<TVoxelRange<FVoxelValue>> Range;
		if (Assets.Num() > 0)
		{
			for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
			{
				auto& Asset = *Assets[Index];

				if (!Asset.Bounds.Intersect(QueryBounds)) continue;

				const auto AssetRangeFlt = Asset.WorldGenerator->GetValueRange_Transform(
					Asset.LocalToWorld,
					Asset.Bounds.Overlap(QueryBounds),
					LOD,
					FVoxelItemStack(ItemHolder, *WorldGenerator, Index));
				const auto AssetRange = TVoxelRange<FVoxelValue>(AssetRangeFlt);

				if (!Range.IsSet())
				{
					Range = AssetRange;
				}
				else
				{
					Range = TVoxelRange<FVoxelValue>::Union(Range.GetValue(), AssetRange);
				}

				if (Asset.Bounds.Contains(QueryBounds))
				{
					// This one is covering everything, no need to continue deeper in the stack nor to check the generator
					return Range.GetValue();
				}
			}
		}
		
		// Note: need to query individual bounds as ItemHolder might be different
		const auto GeneratorRangeFlt = WorldGenerator->GetValueRange(QueryBounds, LOD, FVoxelItemStack(ItemHolder));
		const auto GeneratorRange = TVoxelRange<FVoxelValue>(GeneratorRangeFlt);
		if (!Range.IsSet())
		{
			return GeneratorRange;
		}
		else
		{
			return TVoxelRange<FVoxelValue>::Union(Range.GetValue(), GeneratorRange);
		}
	};
	const auto Reduction = [](auto RangeA, auto RangeB)
	{
		return TVoxelRange<FVoxelValue>::Union(RangeA, RangeB);
	};
	
	// Note: even if WorldBounds doesn't contain InBounds, we don't need to check other values are the queries are always clamped to world bounds
	const auto Result = FVoxelOctreeUtilities::ReduceInBounds<TVoxelRange<FVoxelValue>>(GetOctree(), WorldBounds.Clamp(InBounds), Apply, Reduction);
	
	ensure(Result.IsSet());
	return Result.Get(FVoxelValue::Empty());
}

TVoxelRange<v_flt> FVoxelData::GetCustomOutputRange(TVoxelRange<v_flt> DefaultValue, FName Name, const FVoxelIntBox& InBounds, int32 LOD) const
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	ensure(InBounds.IsValid());
	
	const auto Apply = [&](FVoxelDataOctreeBase& Tree)
	{
		const auto QueryBounds = InBounds.Overlap(Tree.GetBounds());
		
		auto& ItemHolder = Tree.GetItemHolder();
		const auto Assets = ItemHolder.GetItems<FVoxelAssetItem>();

		TOptional<TVoxelRange<v_flt>> Range;
		if (Assets.Num() > 0)
		{
			for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
			{
				auto& Asset = *Assets[Index];

				if (!Asset.Bounds.Intersect(InBounds)) continue;

				const auto AssetRange = Asset.WorldGenerator->GetCustomOutputRange_Transform(
					Asset.LocalToWorld,
					DefaultValue,
					Name,
					InBounds,
					LOD,
					FVoxelItemStack(ItemHolder, *WorldGenerator, Index));

				if (!Range.IsSet())
				{
					Range = AssetRange;
				}
				else
				{
					Range = TVoxelRange<v_flt>::Union(Range.GetValue(), AssetRange);
				}

				if (Asset.Bounds.Contains(QueryBounds))
				{
					// This one is covering everything, no need to continue deeper in the stack nor to check the generator
					return Range.GetValue();
				}
			}
		}
		
		// Note: need to query individual bounds as ItemHolder might be different
		const auto GeneratorRange = WorldGenerator->GetCustomOutputRange(DefaultValue, Name, QueryBounds, LOD, FVoxelItemStack(ItemHolder));
		if (!Range.IsSet())
		{
			return GeneratorRange;
		}
		else
		{
			return TVoxelRange<v_flt>::Union(Range.GetValue(), GeneratorRange);
		}
	};
	const auto Reduction = [](auto RangeA, auto RangeB)
	{
		return TVoxelRange<v_flt>::Union(RangeA, RangeB);
	};
	
	const auto Result = FVoxelOctreeUtilities::ReduceInBounds<TVoxelRange<v_flt>>(GetOctree(), InBounds, Apply, Reduction);
	// TODO: check outside of octree as well?
	return Result.Get(DefaultValue);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetSave(FVoxelUncompressedWorldSaveImpl& OutSave)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelReadScopeLock Lock(*this, FVoxelIntBox::Infinite, "GetSave");

	FVoxelSaveBuilder Builder(Depth);

	TArray<TUniquePtr<TVoxelDataOctreeLeafData<FVoxelValue>>> BuffersToDelete;

	FVoxelOctreeUtilities::IterateAllLeaves(*Octree, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		TVoxelDataOctreeLeafData<FVoxelValue>* ValuesPtr = &Leaf.Values;
		
		if (CVarStoreSpecialValueForGeneratorValuesInSaves.GetValueOnGameThread() != 0)
		{
			VOXEL_ASYNC_SCOPE_COUNTER("Diffing with generator");
			
			// Only if dirty and not compressed to a single value
			if (Leaf.Values.IsDirty() && Leaf.Values.GetDataPtr())
			{
				auto UniquePtr = MakeUnique<TVoxelDataOctreeLeafData<FVoxelValue>>();
				UniquePtr->CreateDataPtr(*this);
				UniquePtr->SetDirty(*this);

				const FVoxelValue* RESTRICT const ExistingDataPtr = Leaf.Values.GetDataPtr();
				FVoxelValue* RESTRICT const NewDataPtr = UniquePtr->GetDataPtr();

				const FVoxelIntBox LeafBounds = Leaf.GetBounds();
				LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
				{
					const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
					const FVoxelValue Value = ExistingDataPtr[Index];
					// Empty stack: items not loaded when loading in LoadFromSave
					const FVoxelValue GeneratorValue = WorldGenerator->Get<FVoxelValue>(X, Y, Z, 0, FVoxelItemStack::Empty);

					if (GeneratorValue == Value)
					{
						NewDataPtr[Index] = FVoxelValue::Special();
					}
					else
					{
						NewDataPtr[Index] = Value;
					}
				});

				UniquePtr->TryCompressToSingleValue(*this);
				ValuesPtr = UniquePtr.Get();
				BuffersToDelete.Emplace(MoveTemp(UniquePtr));
			}
		}
		
		Builder.AddChunk(Leaf.Position, *ValuesPtr, Leaf.Materials, Leaf.Foliage);
	});

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Items");
		
		FScopeLock ItemLock(&ItemsSection);
		for (auto& Item : Items)
		{
			if (Item.IsValid() && Item->ShouldBeSaved())
			{
				Builder.AddPlaceableItem(Item);
			}
		}
	}

	Builder.Save(OutSave);
	
	VOXEL_ASYNC_SCOPE_COUNTER("ClearData");
	for (auto& Buffer : BuffersToDelete)
	{
		// For correct memory reports
		Buffer->ClearData(*this);
	}
}

bool FVoxelData::LoadFromSave(const AVoxelWorld* VoxelWorld, const FVoxelUncompressedWorldSaveImpl& Save, TArray<FVoxelIntBox>& OutBoundsToUpdate)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(VoxelWorld && IsInGameThread());

	{
		FVoxelWriteScopeLock Lock(*this, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		FVoxelOctreeUtilities::IterateEntireTree(*Octree, [&](auto& Tree)
		{
			if (Tree.IsLeafOrHasNoChildren())
			{
				OutBoundsToUpdate.Add(Tree.GetBounds());
			}
		});
	}

	// Will replace the octree
	ClearData();
	ClearDirtyFlag(); // Set by ClearData
	
	FVoxelWriteScopeLock Lock(*this, FVoxelIntBox::Infinite, FUNCTION_FNAME);

	FVoxelSaveLoader Loader(Save);

	int32 ChunkIndex = 0;
	FVoxelOctreeUtilities::IterateEntireTree(*Octree, [&](FVoxelDataOctreeBase& Tree)
	{
		if (ChunkIndex == Loader.NumChunks())
		{
			return;
		}

		const FVoxelIntBox OctreeBounds = Tree.GetBounds();
		const FIntVector CurrentPosition = Loader.GetChunkPosition(ChunkIndex);
		if (Tree.IsLeaf())
		{
			auto& Leaf = Tree.AsLeaf();
			if (CurrentPosition == Tree.Position)
			{
				Loader.ExtractChunk(ChunkIndex, *this, Leaf.Values, Leaf.Materials, Leaf.Foliage);
				
				if (CVarStoreSpecialValueForGeneratorValuesInSaves.GetValueOnGameThread() != 0)
				{
					VOXEL_ASYNC_SCOPE_COUNTER("Loading generator values");
					
					// If we are dirty and we are not a single value, or if we are a single special value
					if (Leaf.Values.IsDirty() && (!Leaf.Values.IsSingleValue() || Leaf.Values.GetSingleValue() == FVoxelValue::Special()))
					{
						if (Leaf.Values.IsSingleValue())
						{
							Leaf.Values.ExpandSingleValue(*this);
						}

						auto* RESTRICT DataPtr = Leaf.Values.GetDataPtr();
						OctreeBounds.Iterate([&](int32 X, int32 Y, int32 Z)
						{
							const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(OctreeBounds.Min, X, Y, Z);
							FVoxelValue& Value = DataPtr[Index];

							if (Value == FVoxelValue::Special())
							{
								// Use the generator value, ignoring all assets and items as they are not loaded
								// The same is done when checking on save
								Value = WorldGenerator->Get<FVoxelValue>(X, Y, Z, 0, FVoxelItemStack::Empty);
							}
						});

						Leaf.Values.TryCompressToSingleValue(*this);
					}
				}

				ChunkIndex++;
				OutBoundsToUpdate.Add(OctreeBounds);
			}
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (OctreeBounds.Contains(CurrentPosition) && !Parent.HasChildren())
			{
				Parent.CreateChildren();
			}
		}
	});
	check(ChunkIndex == Loader.NumChunks() || Save.GetDepth() > Depth);

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Load items");
		for (auto& Item : Loader.GetPlaceableItems(VoxelWorld))
		{
			AddItem(Item.ToSharedRef(), ERecordInHistory::No, true);
		}
	}
	
	return !Loader.GetError();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_UNDO_REDO_IMPL(Return, Dummy) check(IsInGameThread()); if(!ensure(bEnableUndoRedo)) { return Return; }
#define CHECK_UNDO_REDO() CHECK_UNDO_REDO_IMPL({},)
#define CHECK_UNDO_REDO_VOID() CHECK_UNDO_REDO_IMPL(,)

static TAutoConsoleVariable<int32> CVarResetDataChunksWhenUndoingAddItem(
	TEXT("voxel.data.ResetDataChunksWhenUndoingAddItem"),
	0,
	TEXT("If true, will reset all data chunks affected by AddItem when undoing it. If false, these chunks will be left untouched. In both cases, undo is imperfect"),
	ECVF_Default);

bool FVoxelData::Undo(TArray<FVoxelIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	if (HistoryPosition <= 0)
	{
		return false;
	}

	MarkAsDirty();
	HistoryPosition--;

	RedoUniqueIds.Add(CurrentFrameUniqueId);
	CurrentFrameUniqueId = UndoUniqueIds.Pop(false);
	
	{
		FScopeLock ItemLock(&ItemsSection);
		ensure(ItemFrame->IsEmpty());
		if (ItemUndoFrames.Num() > 0 && ItemUndoFrames.Last()->HistoryPosition == HistoryPosition)
		{
			auto UndoFrame = ItemUndoFrames.Pop(false);

			for (auto& Item : UndoFrame->AddedItems)
			{
				FVoxelWriteScopeLock Lock(*this, Item->Bounds, FUNCTION_FNAME);
				FString Error;
				ensureAlwaysMsgf(RemoveItem(
					Item.Get(),
					ERecordInHistory::Yes,
					CVarResetDataChunksWhenUndoingAddItem.GetValueOnGameThread() == 1,
					Error),
					TEXT("Failed to undo add item: %s"), *Error);
				OutBoundsToUpdate.Add(Item->Bounds);
			}
			for (auto& Item : UndoFrame->RemovedItems)
			{
				FVoxelWriteScopeLock Lock(*this, Item->Bounds, FUNCTION_FNAME);
				AddItem(Item.ToSharedRef(), ERecordInHistory::Yes);
				OutBoundsToUpdate.Add(Item->Bounds);
			}

			UndoFrame->HistoryPosition = HistoryPosition + 1;
			ItemRedoFrames.Add(MoveTemp(UndoFrame));
			ItemFrame = MakeUnique<FItemFrame>();
		}
	}
	
	const auto Bounds = UndoFramesBounds.Pop();
	RedoFramesBounds.Add(Bounds);

	LeavesWithRedoStackStack.Emplace();
	auto& LeavesWithRedoStack = LeavesWithRedoStackStack.Last();
	
	FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanUndoRedo<EVoxelUndoRedo::Undo>(HistoryPosition))
		{
			if (Leaf.UndoRedo->GetFramesStack<EVoxelUndoRedo::Redo>().Num() == 0)
			{
				// Only add if this is the first redo to avoid duplicates
#if VOXEL_DEBUG
				for (auto& It : LeavesWithRedoStackStack)
				{
					ensure(!It.Contains(&Leaf));
				}
#endif
				LeavesWithRedoStack.Add(&Leaf);
			}
			Leaf.UndoRedo->UndoRedo<EVoxelUndoRedo::Undo>(*this, Leaf, HistoryPosition);
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});

	return true;
}

bool FVoxelData::Redo(TArray<FVoxelIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	if (HistoryPosition >= MaxHistoryPosition)
	{
		return false;
	}

	MarkAsDirty();
	HistoryPosition++;

	UndoUniqueIds.Add(CurrentFrameUniqueId);
	CurrentFrameUniqueId = RedoUniqueIds.Pop(false);

	{
		FScopeLock ItemLock(&ItemsSection);
		check(ItemFrame->IsEmpty());
		if (ItemRedoFrames.Num() > 0 && ItemRedoFrames.Last()->HistoryPosition == HistoryPosition)
		{
			auto RedoFrame = ItemRedoFrames.Pop(false);

			for (auto& Item : RedoFrame->AddedItems)
			{
				FVoxelWriteScopeLock Lock(*this, Item->Bounds, FUNCTION_FNAME);
				AddItem(Item.ToSharedRef(), ERecordInHistory::Yes);
				OutBoundsToUpdate.Add(Item->Bounds);
			}
			for (auto& Item : RedoFrame->RemovedItems)
			{
				FVoxelWriteScopeLock Lock(*this, Item->Bounds, FUNCTION_FNAME);
				FString Error;
				ensureAlwaysMsgf(RemoveItem(
					Item.Get(),
					ERecordInHistory::Yes,
					CVarResetDataChunksWhenUndoingAddItem.GetValueOnGameThread() == 1,
					Error),
					TEXT("Failed to redo remove item: %s"), *Error);
				OutBoundsToUpdate.Add(Item->Bounds);
			}

			RedoFrame->HistoryPosition = HistoryPosition - 1;
			ItemUndoFrames.Add(MoveTemp(RedoFrame));
			ItemFrame = MakeUnique<FItemFrame>();
		}
	}
	
	const auto Bounds = RedoFramesBounds.Pop();
	UndoFramesBounds.Add(Bounds);

	// We are redoing: pop redo stacks added by the last undo
	if (ensure(LeavesWithRedoStackStack.Num() > 0)) LeavesWithRedoStackStack.Pop(false);
	
	FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanUndoRedo<EVoxelUndoRedo::Redo>(HistoryPosition))
		{
			Leaf.UndoRedo->UndoRedo<EVoxelUndoRedo::Redo>(*this, Leaf, HistoryPosition);
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});

	return true;
}

void FVoxelData::ClearFrames()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO_VOID();

	HistoryPosition = 0;
	MaxHistoryPosition = 0;
	
	UndoFramesBounds.Empty();
	RedoFramesBounds.Empty();

	CurrentFrameUniqueId = 0;

	UndoUniqueIds.Empty();
	RedoUniqueIds.Empty();

	{
		FScopeLock Lock(&ItemsSection);
		ItemFrame = MakeUnique<FItemFrame>();
		ItemUndoFrames.Empty();
		ItemRedoFrames.Empty();
	}

	FVoxelWriteScopeLock Lock(*this, FVoxelIntBox::Infinite, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.UndoRedo.IsValid())
		{
			Leaf.UndoRedo->ClearFrames(Leaf);
		}
	});
}

void FVoxelData::SaveFrame(const FVoxelIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO_VOID();

	{
#if VOXEL_DEBUG
		// Not thread safe, but for debug only so should be ok
		FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.UndoRedo.IsValid() && !Leaf.UndoRedo->IsCurrentFrameEmpty() && !Leaf.GetBounds().Intersect(Bounds))
			{
				ensureMsgf(false, TEXT("Save Frame called on too small bounds! Input Bounds: %s; Leaf Bounds: %s"), *Bounds.ToString(), *Leaf.GetBounds().ToString());
			}
		});
#endif

		// Call SaveFrame on the leaves
		{
			FVoxelReadScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
			FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
			{
				ensureThreadSafe(Leaf.IsLockedForRead());
				if (Leaf.UndoRedo.IsValid())
				{
					Leaf.UndoRedo->SaveFrame(Leaf, HistoryPosition);
				}
			});
		}

		// Clear redo histories
		for (auto& LeavesWithRedoStack : LeavesWithRedoStackStack)
		{
			for (auto* Leaf : LeavesWithRedoStack)
			{
				if (ensure(Leaf->UndoRedo.IsValid()))
				{
					// Note: might be empty if we called Redo already
					// Note: no need to lock, frame stacks are game thread only and a leaf cannot be destroyed without a global lock
					Leaf->UndoRedo->GetFramesStack<EVoxelUndoRedo::Redo>().Reset();
				}
			}
		}
		LeavesWithRedoStackStack.Reset();

#if VOXEL_DEBUG
		// Not thread safe, but for debug only so should be ok
		FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.UndoRedo.IsValid())
			{
				ensure(Leaf.UndoRedo->GetFramesStack<EVoxelUndoRedo::Redo>().Num() == 0);
				ensure(Leaf.UndoRedo->IsCurrentFrameEmpty());
			}
		});
#endif
	}

	// Save items
	{
		FScopeLock Lock(&ItemsSection);
		if (!ItemFrame->IsEmpty())
		{
			ItemFrame->HistoryPosition = HistoryPosition;
			ItemUndoFrames.Add(MoveTemp(ItemFrame));
			ItemFrame = MakeUnique<FItemFrame>();
		}
		ItemRedoFrames.Reset();
	}

	// Important: do all that at the end as HistoryPosition is used above
	
	MarkAsDirty();

	HistoryPosition++;
	MaxHistoryPosition = HistoryPosition;

	UndoFramesBounds.Add(Bounds);
	RedoFramesBounds.Reset();

	UndoUniqueIds.Add(CurrentFrameUniqueId);
	RedoUniqueIds.Reset();
	// Assign new unique id to this frame
	CurrentFrameUniqueId = FrameUniqueIdCounter++;

	ensure(UndoFramesBounds.Num() == HistoryPosition);
	ensure(UndoUniqueIds.Num() == HistoryPosition);
	
}

bool FVoxelData::IsCurrentFrameEmpty()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	{
		FScopeLock Lock(&ItemsSection);
		if (!ItemFrame->IsEmpty())
		{
			return false;
		}
	}

	FVoxelReadScopeLock Lock(*this, FVoxelIntBox::Infinite, "IsCurrentFrameEmpty");
	bool bValue = true;
	FVoxelOctreeUtilities::IterateLeavesByPred(GetOctree(), [&](auto&) { return bValue; }, [&](auto& Leaf)
	{
		if (Leaf.UndoRedo.IsValid())
		{
			bValue &= Leaf.UndoRedo->IsCurrentFrameEmpty();
		}
	});
	return bValue;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelDataWorldGeneratorInstance_AddAssetItem : public TVoxelWorldGeneratorInstanceHelper<FVoxelDataWorldGeneratorInstance_AddAssetItem, UVoxelWorldGenerator>
{
public:
	const FVoxelData& Data;
	explicit FVoxelDataWorldGeneratorInstance_AddAssetItem(const FVoxelData& Data) : Data(Data) {}
	virtual FVector GetUpVector(v_flt, v_flt, v_flt) const override final { return {}; }
	FORCEINLINE v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack&) const
	{
		// Not ideal, but w/e
		return FVoxelDataUtilities::MakeBilinearInterpolatedData(Data).GetValue(X, Y, Z, LOD);
	}
	FORCEINLINE FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack&) const
	{
		return Data.GetMaterial(FMath::RoundToInt(X), FMath::RoundToInt(Y), FMath::RoundToInt(Z), LOD);
	}
	FORCEINLINE TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox&, int32, const FVoxelItemStack&) const
	{
		return TVoxelRange<v_flt>::Infinite();
	}
};

void FVoxelDataUtilities::AddAssetItemDataToLeaf(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf, 
	const FVoxelTransformableWorldGeneratorInstance& WorldGenerator, 
	const FTransform& LocalToWorld, 
	bool bModifyValues, 
	bool bModifyMaterials)
{
	if (bModifyValues)
	{
		Leaf.InitForEdit<FVoxelValue>(Data);
	}
	if (bModifyMaterials)
	{
		Leaf.InitForEdit<FVoxelMaterial>(Data);
	}
	
	const FVoxelDataWorldGeneratorInstance_AddAssetItem PtrWorldGenerator(Data);
	const FVoxelItemStack ItemStack(Leaf.GetItemHolder(), PtrWorldGenerator, 0);

	TVoxelStaticArray<FVoxelValue, VOXELS_PER_DATA_CHUNK> ValuesBuffer;
	TVoxelStaticArray<FVoxelMaterial, VOXELS_PER_DATA_CHUNK> MaterialsBuffer;

	const auto WriteAssetDataToBuffer = [&](auto& Buffer)
	{
		using T = typename std::decay<decltype(Buffer)>::type::ElementType;

		Leaf.GetData<T>().SetDirty(Data);
		
		auto* RESTRICT DataPtr = Leaf.GetData<T>().GetDataPtr();
		check(DataPtr);

		TVoxelQueryZone<T> QueryZone(Leaf.GetBounds(), Buffer.GetData());
		WorldGenerator.Get_Transform<T>(
			LocalToWorld,
			QueryZone,
			0,
			ItemStack);

		if (Data.bEnableMultiplayer)
		{
			for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
			{
				Leaf.Multiplayer->MarkIndexDirty<T>(Index);
			}
		}
		if (Data.bEnableUndoRedo)
		{
			for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
			{
				Leaf.UndoRedo->SavePreviousValue(Index, DataPtr[Index]);
			}
		}
	};
	if (bModifyValues) WriteAssetDataToBuffer(ValuesBuffer);
	if (bModifyMaterials) WriteAssetDataToBuffer(MaterialsBuffer);

	// Need to first write both of them, as the item stack is referencing the data

	if (bModifyValues) FMemory::Memcpy(Leaf.Values.GetDataPtr(), ValuesBuffer.GetData(), VOXELS_PER_DATA_CHUNK * sizeof(FVoxelValue));
	if (bModifyMaterials) FMemory::Memcpy(Leaf.Materials.GetDataPtr(), MaterialsBuffer.GetData(), VOXELS_PER_DATA_CHUNK * sizeof(FVoxelMaterial));
}

void FVoxelData::AddItem(
	const TVoxelSharedRef<FVoxelPlaceableItem>& Item, 
	ERecordInHistory RecordInHistory, 
	bool bDoNotModifyExistingDataChunks)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const int32 MaxPlaceableItemsPerOctree = CVarMaxPlaceableItemsPerOctree.GetValueOnAnyThread();
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Item->Bounds, [&](FVoxelDataOctreeBase& Tree) 
	{
		if (Tree.IsLeaf())
		{
			ensureThreadSafe(Tree.IsLockedForWrite());
			
			Tree.GetItemHolder().AddItem(&Item.Get());
			
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

			if (!bDoNotModifyExistingDataChunks && Item->IsA<FVoxelAssetItem>() && (Leaf.Values.IsDirty() || Leaf.Materials.IsDirty()))
			{
				auto& Asset = *StaticCastSharedRef<FVoxelAssetItem>(Item);
				FVoxelDataUtilities::AddAssetItemDataToLeaf(
					*this,
					Leaf,
					*Asset.WorldGenerator,
					Asset.LocalToWorld,
					Leaf.Values.IsDirty(),
					Leaf.Materials.IsDirty());
			}
		}
		else
		{
			auto& Parent = Tree.AsParent();
			if (!Parent.HasChildren())
			{
				ensureThreadSafe(Parent.IsLockedForWrite());
				if (Tree.GetItemHolder().Num(Item->ItemId) < MaxPlaceableItemsPerOctree)
				{
					Tree.GetItemHolder().AddItem(&Item.Get());
				}
				else
				{
					Parent.CreateChildren();
				}
			}
		}
	});

	FScopeLock Lock(&ItemsSection);
	if (FreeItems.Num() == 0)
	{
		Item->ItemIndex = Items.Add(Item);
	}
	else
	{
		const int32 Index = FreeItems.Pop();
		check(!Items[Index].IsValid());
		Item->ItemIndex = Index;
		Items[Index] = Item;
	}
	if (bEnableUndoRedo && RecordInHistory == ERecordInHistory::Yes)
	{
		ItemFrame->AddedItems.Add(Item);
	}
}

bool FVoxelData::RemoveItem(FVoxelPlaceableItem* Item, ERecordInHistory RecordInHistory, bool bResetOverlappingChunksData, FString& OutError)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	check(Item);

	{
		FScopeLock Lock(&ItemsSection);
		if (!Items.IsValidIndex(Item->ItemIndex))
		{
			OutError = FString::Printf(TEXT("Invalid item: %s"), *Item->GetDescription());
			return false;
		}
		if (!Items[Item->ItemIndex].IsValid())
		{
			OutError = FString::Printf(TEXT("Item already removed: %s"), *Item->GetDescription());
			return false;
		}
	}

	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Item->Bounds, [&](FVoxelDataOctreeBase& Tree) 
	{
		if (Tree.IsLeafOrHasNoChildren())
		{
			ensureThreadSafe(Tree.IsLockedForWrite());

			Tree.GetItemHolder().RemoveItem(Item);

			if (Tree.IsLeaf())
			{
				auto& Leaf = Tree.AsLeaf();
				if (Leaf.Values.GetDataPtr() || Leaf.Values.IsSingleValue())
				{
					if (!Leaf.Values.IsDirty())
					{
						Leaf.Values.ClearData(*this);
					}
					else
					{
						// This is a tricky case: the chunk has been modified by the asset, but other edits have also been applied to it
						// As it's not possible to somehow keep only the other edits, we let the user decide whether they want to reset the chunk entirely or keep all the edits
						// Note: might be possible to do something smarter if UndoRedo is enabled, eg replaying all the edits excluding the AddItem
						if (bResetOverlappingChunksData)
						{
							Leaf.Values.ClearData(*this);
						}
					}
				}
				if (Leaf.Materials.GetDataPtr() || Leaf.Materials.IsSingleValue())
				{
					if (!Leaf.Materials.IsDirty())
					{
						Leaf.Materials.ClearData(*this);
					}
					else
					{
						if (bResetOverlappingChunksData)
						{
							Leaf.Materials.ClearData(*this);
						}
					}
				}
			}
		}
	});
	
	FScopeLock Lock(&ItemsSection);
	auto& ItemPtr = Items[Item->ItemIndex];
	if (bEnableUndoRedo && RecordInHistory == ERecordInHistory::Yes)
	{
		ItemFrame->RemovedItems.Add(ItemPtr);
	}
	ItemPtr.Reset();
	FreeItems.Add(Item->ItemIndex);

	return true;
}