// Copyright 2021 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelData.inl"
#include "VoxelData/VoxelSave.h"
#include "VoxelData/VoxelDataLock.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataUtilities.h"

#include "VoxelDiff.h"
#include "VoxelEnums.h"
#include "VoxelWorld.h"
#include "VoxelQueryZone.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

#include "Misc/ScopeLock.h"
#include "Async/Async.h"

VOXEL_API TAutoConsoleVariable<int32> CVarMaxPlaceableItemsPerOctree(
		TEXT("voxel.data.MaxPlaceableItemsPerOctree"),
		1,
		TEXT("Max number of placeable items per data octree node. If more placeable items are added, the node is split. Low = fast generation, High = very slightly lower memory usage"),
		ECVF_Default);

VOXEL_API TAutoConsoleVariable<int32> CVarStoreSpecialValueForGeneratorValuesInSaves(
		TEXT("voxel.data.StoreSpecialValueForGeneratorValuesInSaves"),
		1,
		TEXT("If true, will store FVoxelValue::Special() instead of the value if it's equal to the generator value when saving. Reduces save size a lot, but increases save time a lot too.\n")
		TEXT("Important: must be the same when saving & loading!"),
		ECVF_Default);

DEFINE_STAT(STAT_NumVoxelAssetItems);
DEFINE_STAT(STAT_NumVoxelDisableEditsItems);
DEFINE_STAT(STAT_NumVoxelDataItems);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline auto CreateGenerator(const AVoxelWorld* World)
{
	auto GeneratorInstance = World->Generator.GetInstance();
	GeneratorInstance->Init(World->GetGeneratorInit());
	return GeneratorInstance;
}

inline int32 ClampDataDepth(int32 Depth)
{
	return FMath::Max(1, FVoxelUtilities::ClampDepth<DATA_CHUNK_SIZE>(Depth));
}

FVoxelDataSettings::FVoxelDataSettings(
	int32 Depth, 
	const TVoxelSharedRef<FVoxelGeneratorInstance>& Generator,
	bool bEnableMultiplayer,
	bool bEnableUndoRedo)
	: Depth(ClampDataDepth(Depth))
	, WorldBounds(FVoxelUtilities::GetBoundsFromDepth<DATA_CHUNK_SIZE>(this->Depth))
	, Generator(Generator)
	, bEnableMultiplayer(bEnableMultiplayer)
	, bEnableUndoRedo(bEnableUndoRedo)
{

}

FVoxelDataSettings::FVoxelDataSettings(
	const FVoxelIntBox& WorldBounds, 
	const TVoxelSharedRef<FVoxelGeneratorInstance>& Generator, 
	bool bEnableMultiplayer, 
	bool bEnableUndoRedo)
	: Depth(ClampDataDepth(FVoxelUtilities::GetOctreeDepthContainingBounds<DATA_CHUNK_SIZE>(WorldBounds)))
	, WorldBounds(WorldBounds)
	, Generator(Generator)
	, bEnableMultiplayer(bEnableMultiplayer)
	, bEnableUndoRedo(bEnableUndoRedo)
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelData::FVoxelData(const FVoxelDataSettings& Settings)
	: IVoxelData(Settings.Depth, Settings.WorldBounds, Settings.bEnableMultiplayer, Settings.bEnableUndoRedo, Settings.Generator.ToSharedRef())
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

TVoxelSharedRef<FVoxelData> FVoxelData::Clone() const
{
	return MakeShareable(new FVoxelData(FVoxelDataSettings(WorldBounds, Generator, bEnableMultiplayer, bEnableUndoRedo)));
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
			Leaf.CustomChannels.ClearData(*this);
		});

		ensure(GetDirtyMemory().Values.GetValue() == 0);
		ensure(GetDirtyMemory().Materials.GetValue() == 0);

		ensure(GetCachedMemory().Values.GetValue() == 0);
		ensure(GetCachedMemory().Materials.GetValue() == 0);

		Octree = MakeUnique<FVoxelDataOctreeParent>(Depth);
	}
	MainLock.Unlock(EVoxelLockType::Write);

	UndoRedo = {};
	MarkAsDirty();

#define CLEAR(Type, Stat) \
	{ \
		auto& ItemsData = GetItemsData<Type>(); \
		FScopeLock Lock(&ItemsData.Section); \
		DEC_DWORD_STAT_BY(Stat, ItemsData.Items.Num()); \
		ItemsData.Items.Reset(); \
	}

	CLEAR(FVoxelAssetItem, STAT_NumVoxelAssetItems);
	CLEAR(FVoxelDisableEditsBoxItem, STAT_NumVoxelDisableEditsItems);
	CLEAR(FVoxelDataItem, STAT_NumVoxelDataItems);

#undef CLEAR
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
		if (bUpdate)
		{
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});
}

template<typename T>
void FVoxelData::CheckIsSingle(const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());
		Leaf.GetData<T>().Compress(*this);
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
			if (Data.HasData())
			{
				VOXEL_SLOW_SCOPE_COUNTER("Copy Data");
				const FIntVector Min = InOctree.GetMin();
				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
				{
					for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
					{
						for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
						{
							const int32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);
							QueryZone.Set(X, Y, Z, Data.Get(Index));
						}
					}
				}
				return;
			}
		}
		
		InOctree.GetFromGeneratorAndAssets<T>(*Generator, QueryZone, LOD);
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

		TOptional<TVoxelRange<FVoxelValue>> Range;
		for (int32 Index = ItemHolder.GetAssetItems().Num() - 1; Index >= 0; Index--)
		{
			auto& Asset = *ItemHolder.GetAssetItems()[Index];

			if (!Asset.Bounds.Intersect(QueryBounds)) continue;

			const auto AssetRangeFlt = Asset.Generator->GetValueRange_Transform(
				Asset.LocalToWorld,
				Asset.Bounds.Overlap(QueryBounds),
				LOD,
				FVoxelItemStack(ItemHolder, *Generator, Index));
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
		
		// Note: need to query individual bounds as ItemHolder might be different
		const auto GeneratorRangeFlt = Generator->GetValueRange(QueryBounds, LOD, FVoxelItemStack(ItemHolder));
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

		TOptional<TVoxelRange<v_flt>> Range;
		for (int32 Index = ItemHolder.GetAssetItems().Num() - 1; Index >= 0; Index--)
		{
			auto& Asset = *ItemHolder.GetAssetItems()[Index];

			if (!Asset.Bounds.Intersect(InBounds)) continue;

			const auto AssetRange = Asset.Generator->GetCustomOutputRange_Transform(
				Asset.LocalToWorld,
				DefaultValue,
				Name,
				InBounds,
				LOD,
				FVoxelItemStack(ItemHolder, *Generator, Index));

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
		
		// Note: need to query individual bounds as ItemHolder might be different
		const auto GeneratorRange = Generator->GetCustomOutputRange(DefaultValue, Name, QueryBounds, LOD, FVoxelItemStack(ItemHolder));
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

void FVoxelData::GetSave(FVoxelUncompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	FVoxelReadScopeLock Lock(*this, FVoxelIntBox::Infinite, "GetSave");

	FVoxelSaveBuilder Builder(Depth);

	TArray<TUniquePtr<TVoxelDataOctreeLeafData<FVoxelValue>>> BuffersToDelete;

	FVoxelOctreeUtilities::IterateAllLeaves(*Octree, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		TVoxelDataOctreeLeafData<FVoxelValue>* ValuesPtr = &Leaf.Values;

#if !ONE_BIT_VOXEL_VALUE
		if (CVarStoreSpecialValueForGeneratorValuesInSaves.GetValueOnGameThread() != 0)
		{
			VOXEL_ASYNC_SCOPE_COUNTER("Diffing with generator");
			
			// Only if dirty and not compressed to a single value
			if (Leaf.Values.IsDirty() && !Leaf.Values.IsSingleValue())
			{
				auto UniquePtr = MakeUnique<TVoxelDataOctreeLeafData<FVoxelValue>>();
				UniquePtr->CreateData(*this);
				UniquePtr->SetIsDirty(true, *this);

				const FVoxelIntBox LeafBounds = Leaf.GetBounds();
				LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
				{
					const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
					const FVoxelValue Value = Leaf.Values.Get(Index);
					// Empty stack: items not loaded when loading in LoadFromSave
					const FVoxelValue GeneratorValue = Generator->Get<FVoxelValue>(X, Y, Z, 0, FVoxelItemStack::Empty);

					if (GeneratorValue == Value)
					{
						UniquePtr->Set(Index, FVoxelValue::Special());
					}
					else
					{
						UniquePtr->Set(Index, Value);
					}
				});

				UniquePtr->TryCompressToSingleValue(*this);
				ValuesPtr = UniquePtr.Get();
				BuffersToDelete.Emplace(MoveTemp(UniquePtr));
			}
		}
#endif
		
		Builder.AddChunk(Leaf.Position, *ValuesPtr, Leaf.Materials);
	});

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Items");
		
		for (auto& Item : AssetItemsData.Items)
		{
			Builder.AddAssetItem(Item->Item);
		}
	}

	Builder.Save(OutSave, OutObjects);
	
	VOXEL_ASYNC_SCOPE_COUNTER("ClearData");
	for (auto& Buffer : BuffersToDelete)
	{
		// For correct memory reports
		Buffer->ClearData(*this);
	}
}

bool FVoxelData::LoadFromSave(const FVoxelUncompressedWorldSaveImpl& Save, const FVoxelPlaceableItemLoadInfo& LoadInfo, TArray<FVoxelIntBox>* OutBoundsToUpdate)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	if (OutBoundsToUpdate)
	{
		FVoxelWriteScopeLock Lock(*this, FVoxelIntBox::Infinite, FUNCTION_FNAME);
		FVoxelOctreeUtilities::IterateEntireTree(*Octree, [&](auto& Tree)
		{
			if (Tree.IsLeafOrHasNoChildren())
			{
				OutBoundsToUpdate->Add(Tree.GetBounds());
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
				Loader.ExtractChunk(ChunkIndex, *this, Leaf.Values, Leaf.Materials);
				
#if !ONE_BIT_VOXEL_VALUE
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

						OctreeBounds.Iterate([&](int32 X, int32 Y, int32 Z)
						{
							const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(OctreeBounds.Min, X, Y, Z);

							if (Leaf.Values.Get(Index) == FVoxelValue::Special())
							{
								// Use the generator value, ignoring all assets and items as they are not loaded
								// The same is done when checking on save
								Leaf.Values.Set(Index, Generator->Get<FVoxelValue>(X, Y, Z, 0, FVoxelItemStack::Empty));
							}
						});

						Leaf.Values.TryCompressToSingleValue(*this);
					}
				}
#endif

				ChunkIndex++;
				if (OutBoundsToUpdate)
				{
					OutBoundsToUpdate->Add(OctreeBounds);
				}
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
		TArray<FVoxelAssetItem> AssetItems;
		Loader.GetPlaceableItems(LoadInfo, AssetItems);
		for (auto& AssetItem : AssetItems)
		{
			AddItem<FVoxelAssetItem, true>(AssetItem);
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

	if (UndoRedo.HistoryPosition <= 0)
	{
		return false;
	}

	MarkAsDirty();
	UndoRedo.HistoryPosition--;

	UndoRedo.RedoUniqueIds.Add(UndoRedo.CurrentFrameUniqueId);
	UndoRedo.CurrentFrameUniqueId = UndoRedo.UndoUniqueIds.Pop(false);
		
	const auto Bounds = UndoRedo.UndoFramesBounds.Pop();
	UndoRedo.RedoFramesBounds.Add(Bounds);

	auto& LeavesWithRedoStack = UndoRedo.LeavesWithRedoStackStack.Emplace_GetRef();
	
	FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanUndoRedo<EVoxelUndoRedo::Undo>(UndoRedo.HistoryPosition))
		{
			if (Leaf.UndoRedo->GetFramesStack<EVoxelUndoRedo::Redo>().Num() == 0)
			{
				// Only add if this is the first redo to avoid duplicates
#if VOXEL_DEBUG
				for (auto& It : UndoRedo.LeavesWithRedoStackStack)
				{
					ensure(!It.Contains(&Leaf));
				}
#endif
				LeavesWithRedoStack.Add(&Leaf);
			}
			Leaf.UndoRedo->UndoRedo<EVoxelUndoRedo::Undo>(*this, Leaf, UndoRedo.HistoryPosition);
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});

	return true;
}

bool FVoxelData::Redo(TArray<FVoxelIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	if (UndoRedo.HistoryPosition >= UndoRedo.MaxHistoryPosition)
	{
		return false;
	}

	MarkAsDirty();
	UndoRedo.HistoryPosition++;

	UndoRedo.UndoUniqueIds.Add(UndoRedo.CurrentFrameUniqueId);
	UndoRedo.CurrentFrameUniqueId = UndoRedo.RedoUniqueIds.Pop(false);
	
	const auto Bounds = UndoRedo.RedoFramesBounds.Pop();
	UndoRedo.UndoFramesBounds.Add(Bounds);

	// We are redoing: pop redo stacks added by the last undo
	if (ensure(UndoRedo.LeavesWithRedoStackStack.Num() > 0)) UndoRedo.LeavesWithRedoStackStack.Pop(false);
	
	FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanUndoRedo<EVoxelUndoRedo::Redo>(UndoRedo.HistoryPosition))
		{
			Leaf.UndoRedo->UndoRedo<EVoxelUndoRedo::Redo>(*this, Leaf, UndoRedo.HistoryPosition);
			OutBoundsToUpdate.Add(Leaf.GetBounds());
		}
	});

	return true;
}

void FVoxelData::ClearFrames()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO_VOID();

	UndoRedo = {};

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
					Leaf.UndoRedo->SaveFrame(Leaf, UndoRedo.HistoryPosition);
				}
			});
		}

		// Clear redo histories
		for (auto& LeavesWithRedoStack : UndoRedo.LeavesWithRedoStackStack)
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
		UndoRedo.LeavesWithRedoStackStack.Reset();

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

	// Important: do all that at the end as HistoryPosition is used above
	
	MarkAsDirty();

	UndoRedo.HistoryPosition++;
	UndoRedo.MaxHistoryPosition = UndoRedo.HistoryPosition;

	UndoRedo.UndoFramesBounds.Add(Bounds);
	UndoRedo.RedoFramesBounds.Reset();

	UndoRedo.UndoUniqueIds.Add(UndoRedo.CurrentFrameUniqueId);
	UndoRedo.RedoUniqueIds.Reset();
	// Assign new unique id to this frame
	UndoRedo.CurrentFrameUniqueId = UndoRedo.FrameUniqueIdCounter++;

	ensure(UndoRedo.UndoFramesBounds.Num() == UndoRedo.HistoryPosition);
	ensure(UndoRedo.UndoUniqueIds.Num() == UndoRedo.HistoryPosition);
}

bool FVoxelData::IsCurrentFrameEmpty()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

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

class FVoxelDataGeneratorInstance_AddAssetItem : public TVoxelGeneratorInstanceHelper<FVoxelDataGeneratorInstance_AddAssetItem, UVoxelGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelDataGeneratorInstance_AddAssetItem, UVoxelGenerator>;
	
	const FVoxelData& Data;
	explicit FVoxelDataGeneratorInstance_AddAssetItem(const FVoxelData& Data)
		: Super(nullptr)
		, Data(Data)
	{
	}
	
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

void FVoxelDataUtilities::AddAssetItemToLeafData(
	const FVoxelData& Data,
	FVoxelDataOctreeLeaf& Leaf, 
	const FVoxelTransformableGeneratorInstance& Generator, 
	const FVoxelIntBox& Bounds,
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
	
	const FVoxelIntBox LeafBounds = Leaf.GetBounds();
	const FVoxelIntBox BoundsToEdit = LeafBounds.Overlap(Bounds);
	
	const FVoxelDataGeneratorInstance_AddAssetItem PtrGenerator(Data);
	const FVoxelItemStack ItemStack(Leaf.GetItemHolder(), PtrGenerator, 0);

	TVoxelArrayFwd<FVoxelValue> ValuesBuffer;
	TVoxelArrayFwd<FVoxelMaterial> MaterialsBuffer;

	const auto WriteAssetDataToBuffer = [&](auto& Buffer, auto TypeDecl)
	{
		using T = decltype(TypeDecl);

		Buffer.SetNumUninitialized(BoundsToEdit.Count());

		Leaf.GetData<T>().SetIsDirty(true, Data);
		
		TVoxelQueryZone<T> QueryZone(BoundsToEdit, Buffer);
		Generator.Get_Transform<T>(
			LocalToWorld,
			QueryZone,
			0,
			ItemStack);
	};
	if (bModifyValues) WriteAssetDataToBuffer(ValuesBuffer, FVoxelValue());
	if (bModifyMaterials) WriteAssetDataToBuffer(MaterialsBuffer, FVoxelMaterial());

	// Need to first write both of them, as the item stack is referencing the data

	const FIntVector Size = BoundsToEdit.Size();
	FVoxelDataOctreeSetter::Set<FVoxelValue, FVoxelMaterial>(Data, Leaf, [&](auto Lambda) { BoundsToEdit.Iterate(Lambda); }, 
		[&](int32 X, int32 Y, int32 Z, FVoxelValue& Value, FVoxelMaterial& Material)
		{
			const int32 Index = FVoxelUtilities::Get3DIndex(Size, X, Y, Z, BoundsToEdit.Min);
			if (bModifyValues)
			{
				Value = FVoxelUtilities::Get(ValuesBuffer, Index);
			}
			if (bModifyMaterials)
			{
				Material = FVoxelUtilities::Get(MaterialsBuffer, Index);
			}
		});
}