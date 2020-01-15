// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelWorld.h"
#include "StackArray.h"

#include "Misc/ScopeLock.h"
#include "Async/Async.h"

TAutoConsoleVariable<int32> CVarMaxPlaceableItemsPerOctree(
		TEXT("voxel.data.MaxPlaceableItemsPerOctree"),
		32,
		TEXT("Max number of placeable items per data octree node. If more placeable items are added, the node is split"),
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

FVoxelDataSettings::FVoxelDataSettings(const AVoxelWorld* World, EVoxelPlayType PlayType)
	: Depth(FVoxelUtilities::ClampDataDepth(FVoxelUtilities::GetDataDepthFromChunkDepth(World->OctreeDepth)))
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
	: Depth(Depth)
	, WorldBounds(FVoxelUtilities::GetBoundsFromDepth<DATA_CHUNK_SIZE>(Depth))
	, WorldGenerator(WorldGenerator)
	, bEnableMultiplayer(bEnableMultiplayer)
	, bEnableUndoRedo(bEnableUndoRedo)
{

}

FVoxelDataSettings::FVoxelDataSettings(
	const FIntBox& WorldBounds, 
	const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& WorldGenerator, 
	bool bEnableMultiplayer, 
	bool bEnableUndoRedo)
	: Depth(FVoxelUtilities::GetOctreeDepthContainingBounds<DATA_CHUNK_SIZE>(WorldBounds))
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
	: Depth(Settings.Depth)
	, WorldBounds(Settings.WorldBounds)
	, bEnableMultiplayer(Settings.bEnableMultiplayer)
	, bEnableUndoRedo(Settings.bEnableUndoRedo)
	, WorldGenerator(Settings.WorldGenerator)
	, Octree(MakeUnique<FVoxelDataOctreeParent>(Depth))
{
	check(Depth > 0);
	check(Octree->GetBounds().Contains(WorldBounds));
}

TVoxelSharedRef<FVoxelData> FVoxelData::Create(const FVoxelDataSettings& Settings, int32 DataOctreeInitialSubdivisionDepth)
{
	auto* Data = new FVoxelData(Settings);

	{
		VOXEL_SCOPE_COUNTER("Subdivide Data");
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelDataOctreeLocker
{
public:
	const EVoxelLockType LockType;
	const FIntBox Bounds;
	const FName Name;

	FVoxelDataOctreeLocker(EVoxelLockType LockType, const FIntBox& Bounds, FName Name)
		: LockType(LockType)
		, Bounds(Bounds)
		, Name(Name)
	{
	}

	TArray<FVoxelOctreeId> Lock(FVoxelDataOctreeBase& Octree)
	{
		VOXEL_FUNCTION_COUNTER();
		
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
		VOXEL_FUNCTION_COUNTER();
		
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

TUniquePtr<FVoxelDataLockInfo> FVoxelData::Lock(EVoxelLockType LockType, const FIntBox& Bounds, FName Name) const
{
	VOXEL_FUNCTION_COUNTER();
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
	VOXEL_FUNCTION_COUNTER();

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
	VOXEL_FUNCTION_COUNTER();
	
	MainLock.Lock(EVoxelLockType::Write);
	Octree = MakeUnique<FVoxelDataOctreeParent>(Depth);
	MainLock.Unlock(EVoxelLockType::Write);

	HistoryPosition = 0;
	MaxHistoryPosition = 0;
	UndoFramesBounds.Reset();
	RedoFramesBounds.Reset();
	bIsDirty = true;
	
	FScopeLock Lock(&ItemsSection);
	FreeItems.Empty();
	Items.Empty();
	ItemFrame = MakeUnique<FItemFrame>();
	ItemUndoFrames.Empty();
	ItemRedoFrames.Empty();
}

void FVoxelData::ClearOctreeData(TArray<FIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateEntireTree(GetOctree(), [&](FVoxelDataOctreeBase& Chunk)
	{
		if (Chunk.IsLeaf())
		{
			ensureThreadSafe(Chunk.IsLockedForWrite());
			auto& Leaf = Chunk.AsLeaf();
			bool bUpdate = false;
			if (Leaf.GetData<FVoxelValue>().IsDirty())
			{
				bUpdate = true;
				Leaf.GetData<FVoxelValue>().ClearData();
			}
			if (Leaf.GetData<FVoxelMaterial>().IsDirty())
			{
				bUpdate = true;
				Leaf.GetData<FVoxelMaterial>().ClearData();
			}
			if (bUpdate)
			{
				OutBoundsToUpdate.Add(Leaf.GetBounds());
			}
		}
	});
}

template<typename T>
void FVoxelData::CacheBounds(const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (Chunk.IsLeaf())
		{
			ensureThreadSafe(Chunk.IsLockedForWrite());

			auto& Leaf = Chunk.AsLeaf();
			auto& DataHolder = Leaf.GetData<T>();
			if (!DataHolder.GetDataPtr() && !DataHolder.IsSingleValue())
			{
				DataHolder.CreateDataPtr();
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

template VOXEL_API void FVoxelData::CacheBounds<FVoxelValue   >(const FIntBox&);
template VOXEL_API void FVoxelData::CacheBounds<FVoxelMaterial>(const FIntBox&);

template<typename T>
void FVoxelData::ClearCacheInBounds(const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Chunk)
	{
		if (Chunk.IsLeaf())
		{
			ensureThreadSafe(Chunk.IsLockedForWrite());

			auto& DataHolder = Chunk.AsLeaf().GetData<T>();
			if (DataHolder.GetDataPtr() && !DataHolder.IsDirty())
			{
				DataHolder.ClearData();
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

template VOXEL_API void FVoxelData::ClearCacheInBounds<FVoxelValue   >(const FIntBox&);
template VOXEL_API void FVoxelData::ClearCacheInBounds<FVoxelMaterial>(const FIntBox&);

template<typename T>
void FVoxelData::CheckIsSingle(const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());

		Leaf.GetData<T>().TryCompressToSingleValue();
	});
}

template VOXEL_API void FVoxelData::CheckIsSingle<FVoxelValue   >(const FIntBox&);
template VOXEL_API void FVoxelData::CheckIsSingle<FVoxelMaterial>(const FIntBox&);

template<typename T>
void FVoxelData::Get(TVoxelQueryZone<T>& GlobalQueryZone, int32 LOD) const
{
	VOXEL_FUNCTION_COUNTER();

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

	// Handle data outside of the octree bounds (happens on edges with marching cubes, as it's querying N + 1 voxels with N a power of 2)
	const FIntBox OctreeBounds = Octree->GetBounds();
	check(OctreeBounds.IsMultipleOf(GlobalQueryZone.Step));
	if (!OctreeBounds.Contains(GlobalQueryZone.Bounds))
	{
		for (auto& LocalBounds : GlobalQueryZone.Bounds.Difference(OctreeBounds))
		{
			check(LocalBounds.IsMultipleOf(GlobalQueryZone.Step));
			auto LocalQueryZone = GlobalQueryZone.ShrinkTo(LocalBounds);
			WorldGenerator->Get(LocalQueryZone, LOD, FVoxelItemStack::Empty);
		}
	}
}

template VOXEL_API void FVoxelData::Get<FVoxelValue   >(TVoxelQueryZone<FVoxelValue   >&, int32) const;
template VOXEL_API void FVoxelData::Get<FVoxelMaterial>(TVoxelQueryZone<FVoxelMaterial>&, int32) const;

TVoxelRange<FVoxelValue> FVoxelData::GetValueRange(const FIntBox& Bounds, int32 LOD) const
{
	VOXEL_FUNCTION_COUNTER();
	ensure(Bounds.IsValid());
	
	const auto Apply = [&](FVoxelDataOctreeBase& Tree)
	{
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
			const auto QueryBounds = Bounds.Overlap(Tree.GetBounds());
			for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
			{
				auto& Asset = *Assets[Index];

				if (!Asset.Bounds.Intersect(Bounds)) continue;

				const auto AssetRangeFlt = Asset.WorldGenerator->GetValueRange_Transform(
					Asset.LocalToWorld,
					Asset.Bounds.Overlap(Bounds),
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
		
		const auto GeneratorRangeFlt = WorldGenerator->GetValueRange(Bounds, LOD, FVoxelItemStack(ItemHolder));
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
	
	const FIntBox OctreeBounds = Octree->GetBounds();
	auto Result = FVoxelOctreeUtilities::ReduceInBounds<TVoxelRange<FVoxelValue>>(GetOctree(), Bounds.Overlap(OctreeBounds), Apply, Reduction);
	
	if (!OctreeBounds.Contains(Bounds))
	{
		for (auto& LocalBounds : Bounds.Difference(OctreeBounds))
		{
			const auto LocalRange = TVoxelRange<FVoxelValue>(WorldGenerator->GetValueRange(LocalBounds, LOD, FVoxelItemStack::Empty));
			Result = Result.IsSet() ? TVoxelRange<FVoxelValue>::Union(Result.GetValue(), LocalRange) : LocalRange;
		}
	}

	ensure(Result.IsSet());
	return Result.Get(FVoxelValue::Empty());
}

TVoxelRange<v_flt> FVoxelData::GetCustomOutputRange(TVoxelRange<v_flt> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD) const
{
	VOXEL_FUNCTION_COUNTER();
	
	const auto Apply = [&](FVoxelDataOctreeBase& Tree)
	{
		auto& ItemHolder = Tree.GetItemHolder();
		const auto Assets = ItemHolder.GetItems<FVoxelAssetItem>();

		TOptional<TVoxelRange<v_flt>> Range;
		if (Assets.Num() > 0)
		{
			const auto QueryBounds = Bounds.Overlap(Tree.GetBounds());
			for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
			{
				auto& Asset = *Assets[Index];

				if (!Asset.Bounds.Intersect(Bounds)) continue;

				const auto AssetRange = Asset.WorldGenerator->GetCustomOutputRange_Transform(
					Asset.LocalToWorld,
					DefaultValue,
					Name,
					Bounds,
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
		
		const auto GeneratorRange = WorldGenerator->GetValueRange(Bounds, LOD, FVoxelItemStack(ItemHolder));
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
	
	const auto Result = FVoxelOctreeUtilities::ReduceInBounds<TVoxelRange<v_flt>>(GetOctree(), Bounds, Apply, Reduction);
	return Result.Get(DefaultValue);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetSave(FVoxelUncompressedWorldSave& OutSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(IsInGameThread());

	FVoxelReadScopeLock Lock(*this, FIntBox::Infinite, "GetSave");

	FVoxelSaveBuilder Builder(Depth);

	FVoxelOctreeUtilities::IterateAllLeaves(*Octree, [&](FVoxelDataOctreeLeaf& Leaf)
	{
		Builder.AddChunk(Leaf.Position, Leaf.Values, Leaf.Materials, Leaf.Foliage);
	});

	{
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
}

bool FVoxelData::LoadFromSave(const AVoxelWorld* VoxelWorld, const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	
	check(VoxelWorld && IsInGameThread());

	{
		FVoxelWriteScopeLock Lock(*this, FIntBox::Infinite, FUNCTION_FNAME);
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
	bIsDirty = false; // Set by ClearData
	
	FVoxelWriteScopeLock Lock(*this, FIntBox::Infinite, FUNCTION_FNAME);

	FVoxelSaveLoader Loader(Save);

	int32 Index = 0;
	FVoxelOctreeUtilities::IterateEntireTree(*Octree, [&](FVoxelDataOctreeBase& Tree)
	{
		if (Index == Loader.NumChunks())
		{
			return;
		}

		const FIntBox OctreeBounds = Tree.GetBounds();
		const FIntVector CurrentPosition = Loader.GetChunkPosition(Index);
		if (Tree.IsLeaf())
		{
			auto& Leaf = Tree.AsLeaf();
			if (CurrentPosition == Tree.Position)
			{
				Loader.ExtractChunk(Index, Leaf.Values, Leaf.Materials, Leaf.Foliage);

				Index++;
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
	check(Index == Loader.NumChunks() || Save.GetDepth() > Depth);

	for (auto& Item : Loader.GetPlaceableItems(VoxelWorld))
	{
		AddItem(Item.ToSharedRef(), ERecordInHistory::No, true);
	}
	
	return !Loader.GetError();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_UNDO_REDO_IMPL(r) check(IsInGameThread()); if(!ensure(bEnableUndoRedo)) { return r; }
#define NO_ARG
#define CHECK_UNDO_REDO() CHECK_UNDO_REDO_IMPL(NO_ARG)
#define CHECK_UNDO_REDO_BOOL() CHECK_UNDO_REDO_IMPL(false)

static TAutoConsoleVariable<int32> CVarResetDataChunksWhenUndoingAddItem(
	TEXT("voxel.data.ResetDataChunksWhenUndoingAddItem"),
	0,
	TEXT("If true, will reset all data chunks affected by AddItem when undoing it. If false, these chunks will be left untouched. In both cases, undo is imperfect"),
	ECVF_Default);

void FVoxelData::Undo(TArray<FIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	if (HistoryPosition > 0)
	{
		MarkAsDirty();
		HistoryPosition--;

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
		FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
		FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanUndo(HistoryPosition))
			{
				if (Leaf.Values.IsSingleValue()) Leaf.Values.ExpandSingleValue();
				if (Leaf.Materials.IsSingleValue()) Leaf.Materials.ExpandSingleValue();
				if (Leaf.Foliage.IsSingleValue()) Leaf.Foliage.ExpandSingleValue();

				// Note: some data ptrs might be null if we haven't edited them yet
				
				Leaf.UndoRedo->Undo(Leaf.Values.GetDataPtr(), Leaf.Materials.GetDataPtr(), Leaf.Foliage.GetDataPtr(), HistoryPosition);
				OutBoundsToUpdate.Add(Leaf.GetBounds());
			}
		});
	}
}

void FVoxelData::Redo(TArray<FIntBox>& OutBoundsToUpdate)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	if (HistoryPosition < MaxHistoryPosition)
	{
		MarkAsDirty();
		HistoryPosition++;

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
		FVoxelWriteScopeLock Lock(*this, Bounds, FUNCTION_FNAME);
		FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.UndoRedo.IsValid() && Leaf.UndoRedo->CanRedo(HistoryPosition))
			{
				if (Leaf.Values.IsSingleValue()) Leaf.Values.ExpandSingleValue();
				if (Leaf.Materials.IsSingleValue()) Leaf.Materials.ExpandSingleValue();
				if (Leaf.Foliage.IsSingleValue()) Leaf.Foliage.ExpandSingleValue();

				// Note: some data ptrs might be null if we haven't edited them yet
				
				Leaf.UndoRedo->Redo(Leaf.Values.GetDataPtr(), Leaf.Materials.GetDataPtr(), Leaf.Foliage.GetDataPtr(), HistoryPosition);
				OutBoundsToUpdate.Add(Leaf.GetBounds());
			}
		});
	}
}

void FVoxelData::ClearFrames()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	HistoryPosition = 0;
	MaxHistoryPosition = 0;
	UndoFramesBounds.Reset();
	RedoFramesBounds.Reset();

	{
		FScopeLock Lock(&ItemsSection);
		ItemFrame = MakeUnique<FItemFrame>();
		ItemUndoFrames.Empty();
		ItemRedoFrames.Empty();
	}

	FVoxelWriteScopeLock Lock(*this, FIntBox::Infinite, FUNCTION_FNAME);
	FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](auto& Leaf)
	{
		if (Leaf.UndoRedo.IsValid())
		{
			Leaf.UndoRedo->ClearFrames();
		}
	});
}

void FVoxelData::SaveFrame(const FIntBox& Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO();

	MarkAsDirty();
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
	FVoxelOctreeUtilities::IterateLeavesInBounds(GetOctree(), Bounds, [&](auto& Leaf)
	{
		if (Leaf.UndoRedo.IsValid())
		{
			Leaf.UndoRedo->SaveFrame(HistoryPosition);
		}
	});

#if VOXEL_DEBUG
	// Not thread safe, but for debug only so should be ok
	FVoxelOctreeUtilities::IterateAllLeaves(GetOctree(), [&](auto& Leaf)
	{
		if (Leaf.UndoRedo.IsValid())
		{
			ensure(Leaf.UndoRedo->IsCurrentFrameEmpty());
		}
	});
#endif

	HistoryPosition++;
	MaxHistoryPosition = HistoryPosition;

	UndoFramesBounds.Add(Bounds);
	RedoFramesBounds.Reset();

	ensure(UndoFramesBounds.Num() == HistoryPosition);
}

bool FVoxelData::IsCurrentFrameEmpty()
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_UNDO_REDO_BOOL();

	{
		FScopeLock Lock(&ItemsSection);
		if (!ItemFrame->IsEmpty())
		{
			return false;
		}
	}

	FVoxelReadScopeLock Lock(*this, FIntBox::Infinite, "IsCurrentFrameEmpty");
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
	FORCEINLINE TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox&, int32, const FVoxelItemStack&) const
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
		Leaf.InitForEdit<FVoxelValue>(*Data.WorldGenerator, Data.bEnableMultiplayer, Data.bEnableUndoRedo);
	}
	if (bModifyMaterials)
	{
		Leaf.InitForEdit<FVoxelMaterial>(*Data.WorldGenerator, Data.bEnableMultiplayer, Data.bEnableUndoRedo);
	}
	
	const FVoxelDataWorldGeneratorInstance_AddAssetItem PtrWorldGenerator(Data);
	const FVoxelItemStack ItemStack(Leaf.GetItemHolder(), PtrWorldGenerator, 0);

	TStackArray<FVoxelValue, VOXELS_PER_DATA_CHUNK> ValuesBuffer;
	TStackArray<FVoxelMaterial, VOXELS_PER_DATA_CHUNK> MaterialsBuffer;

	const auto WriteAssetDataToBuffer = [&](auto& Buffer)
	{
		using T = typename std::decay<decltype(Buffer)>::type::ElementType;
		
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
	VOXEL_FUNCTION_COUNTER();

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
				Leaf.Values.ClearData();
			}
			if (!Leaf.Materials.IsDirty())
			{
				Leaf.Materials.ClearData();
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
	VOXEL_FUNCTION_COUNTER();
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
						Leaf.Values.ClearData();
					}
					else
					{
						// This is a tricky case: the chunk has been modified by the asset, but other edits have also been applied to it
						// As it's not possible to somehow keep only the other edits, we let the user decide whether they want to reset the chunk entirely or keep all the edits
						// Note: might be possible to do something smarter if UndoRedo is enabled, eg replaying all the edits excluding the AddItem
						if (bResetOverlappingChunksData)
						{
							Leaf.Values.ClearData();
						}
					}
				}
				if (Leaf.Materials.GetDataPtr() || Leaf.Materials.IsSingleValue())
				{
					if (!Leaf.Materials.IsDirty())
					{
						Leaf.Materials.ClearData();
					}
					else
					{
						if (bResetOverlappingChunksData)
						{
							Leaf.Materials.ClearData();
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