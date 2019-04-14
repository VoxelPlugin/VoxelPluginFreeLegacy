// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelDataOctree.h"
#include "Misc/ScopeLock.h"
#include "HAL/Platform.h"

#include "VoxelWorldGenerator.h"
#include "VoxelMathUtilities.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

bool FVoxelDataOctree::AreBoundsCached(const FIntBox& Bounds)
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		return true;
	}

	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());
		if (LOD == 0)
		{
			return IsCached();
		}
		else
		{
			return false;
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			if (!Child.AreBoundsCached(Bounds))
			{
				return false;
			}
		}
		return true;
	}
}

void FVoxelDataOctree::CacheBounds(const FIntBox& Bounds, bool bIsManualCache, bool bCacheValues, bool bCacheMaterials)
{
	if (!Bounds.Intersect(OctreeBounds))
	{
		return;
	}

	if (LOD == 0)
	{
		ensureThreadSafe(IsLockedForWrite());
		Cache(bIsManualCache, bCacheValues, bCacheMaterials);
	}
	else
	{
		if (IsLeaf())
		{
			CreateChildren();
		}
		for (auto& Child : GetChildren())
		{
			Child.CacheBounds(Bounds, bIsManualCache, bCacheValues, bCacheMaterials);
		}
	}
}

void FVoxelDataOctree::Cache(bool bIsManualCache, bool bCacheValues, bool bCacheMaterials)
{
	ensureThreadSafe(IsLockedForWrite());
	check(LOD == 0);
	bIsManuallyCached = IsCreated() ? bIsManualCache || bIsManuallyCached : bIsManualCache; // Manual cache after auto: manual; auto after manual: manual
	if (!IsCreated())
	{
		Create();
	}
	CreateArrayAndInitFromWorldGenerator(bCacheValues && !Cell->GetArray<FVoxelValue>(), bCacheMaterials && !Cell->GetArray<FVoxelMaterial>());
}

void FVoxelDataOctree::ClearCache()
{
	ensureThreadSafe(IsLockedForWrite());
	check(LOD == 0);
	check(!IsManuallyCached());

	if (IsCreated() && IsCacheOnly())
	{
		Destroy();
	}
}

void FVoxelDataOctree::ClearManualCache()
{
	ensureThreadSafe(IsLockedForWrite());
	check(IsManuallyCached());
	bIsManuallyCached = false;
}

void FVoxelDataOctree::GetOctreesToCacheAndExistingCachedOctrees(
	uint32 Time,
	uint32 Threshold,
	TArray<CacheElement>& OutOctreesToCacheAndCachedOctrees,
	TArray<FVoxelDataOctree*>& OutOctreesToSubdivide)
{
	if (NumberOfWorldGeneratorReadsSinceLastCache > 0)
	{
		ensure(LastAccessTime <= Time);
		LastAccessTime = Time;
	}

	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());
		if (LOD == 0 && IsCached())
		{
			if (!IsManuallyCached())
			{
				OutOctreesToCacheAndCachedOctrees.Add(CacheElement{ true, GetCachePriority(), this });
			}
		}
		else
		{
			if (ShouldBeCached(Threshold))
			{
				if (LOD == 0)
				{
					OutOctreesToCacheAndCachedOctrees.Add(CacheElement{ false, GetCachePriority(), this });
				}
				else
				{
					OutOctreesToSubdivide.Add(this);
				}
			}
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.GetOctreesToCacheAndExistingCachedOctrees(Time, Threshold, OutOctreesToCacheAndCachedOctrees, OutOctreesToSubdivide);
		}
	}

	NumberOfWorldGeneratorReadsSinceLastCache = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::GetMap(const FIntBox& Bounds, FVoxelMap& OutMap) const
{
	if (Bounds.Intersect(GetBounds()))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForRead());
			if (LOD == 0 && IsCreated())
			{
				OutMap.Add(GetBounds().Min / VOXEL_CELL_SIZE, this);
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.GetMap(Bounds, OutMap);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EVoxelEmptyState FVoxelDataOctree::IsEmpty(const FIntBox& Bounds, int32 InLOD) const
{
	check(Bounds.IsMultipleOf(1 << InLOD));

	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());
		check(ItemHolder);

		if (LOD == 0)
		{
			if (IsCreated() && !IsCacheOnly())
			{
				return EVoxelEmptyState::Unknown;
			}
		}
		for (auto& Items : ItemHolder->GetAllItems())
		{
			for (auto& Item : Items)
			{
				if (Item->Bounds.Intersect(Bounds))
				{
					return EVoxelEmptyState::Unknown;
				}
			}
		}
		return WorldGenerator->IsEmpty(Bounds, InLOD);
	}
	else
	{
		EVoxelEmptyState TmpEmptyState = EVoxelEmptyState::Unknown;
		for (auto& Child : GetChildren())
		{
			if (Child.GetBounds().Intersect(Bounds))
			{
				auto ChildEmptyState = Child.IsEmpty(Bounds, InLOD);
				if (TmpEmptyState == EVoxelEmptyState::Unknown)
				{
					// First init
					TmpEmptyState = ChildEmptyState;
				}
				else if (TmpEmptyState != ChildEmptyState)
				{
					TmpEmptyState = EVoxelEmptyState::Unknown;
				}

				if (TmpEmptyState == EVoxelEmptyState::Unknown)
				{
					break;
				}
			}
		}
		return TmpEmptyState;
	}
}

void FVoxelDataOctree::GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& InQueryZone, int32 QueryLOD)
{
	if (!InQueryZone.Bounds.Intersect(OctreeBounds))
	{
		return;
	}

	auto QueryZone = InQueryZone.ShrinkTo(OctreeBounds);

	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());

		if (QueryLOD <= MAX_LOD_USED_FOR_CACHE)
		{
			NumberOfWorldGeneratorReadsSinceLastCache++;
		}
	
		if (LOD > 0 || !IsCreated())
		{
			WorldGenerator->GetValuesAndMaterials(Values, Materials, QueryZone, QueryLOD, *ItemHolder);
		}
		else
		{
			auto* CellValues = Cell->GetArray<FVoxelValue>();
			auto* CellMaterials = Cell->GetArray<FVoxelMaterial>();

			for (int32 Z : QueryZone.ZIt())
			{
				for (int32 Y : QueryZone.YIt())
				{
					for (int32 X : QueryZone.XIt())
					{
						int32 Index = QueryZone.GetIndex(X, Y, Z);
						int32 CellIndex = IndexFromGlobalCoordinates(X, Y, Z);

						if (Values && CellValues)
						{
							Values[Index] = CellValues[CellIndex];
						}
						if (Materials && CellMaterials)
						{
							Materials[Index] = CellMaterials[CellIndex];
						}
					}
				}
			}

			if ((!CellValues && Values) || (!CellMaterials && Materials))
			{
				WorldGenerator->GetValuesAndMaterials(CellValues ? nullptr : Values, CellMaterials ? nullptr : Materials, QueryZone, QueryLOD, *ItemHolder);
			}
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			if (Child.OctreeBounds.Intersect(QueryZone.Bounds))
			{
				Child.GetValuesAndMaterials(Values, Materials, QueryZone, QueryLOD);
			}
		}
	}
}

void FVoxelDataOctree::ClearData()
{	
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForWrite());
		check(ItemHolder);
		ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>();

		if (LOD == 0 && IsCreated())
		{
			Destroy();
		}
	}
	else
	{
		check(!ItemHolder);
		for (auto& Child : GetChildren())
		{
			Child.ClearData();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::GetCreatedChunksOverlappingBox(const FIntBox& Box, TArray<FVoxelDataOctree*>& OutOctrees)
{
	if (GetBounds().Intersect(Box))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForRead());
			if (LOD == 0 && IsCreated())
			{
				OutOctrees.Add(this);
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.GetCreatedChunksOverlappingBox(Box, OutOctrees);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::Save(FVoxelSaveBuilder& Builder)
{
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());
		if (LOD == 0 && IsCreated())
		{
			Builder.AddChunk(Position,
				Cell->IsArrayDirty<FVoxelValue   >() ? Cell->GetArray<FVoxelValue   >() : nullptr,
				Cell->IsArrayDirty<FVoxelMaterial>() ? Cell->GetArray<FVoxelMaterial>() : nullptr);
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.Save(Builder);
		}
	}
}

void FVoxelDataOctree::Load(int32& Index, const FVoxelSaveLoader& Loader, TArray<FIntBox>& OutBoundsToUpdate)
{
	if (Index == Loader.NumChunks())
	{
		return;
	}

	FIntVector CurrentPosition = Loader.GetChunkPosition(Index);
	if (LOD == 0)
	{
		ensureThreadSafe(IsLockedForWrite());
		if (CurrentPosition == Position)
		{
			if (!IsCreated())
			{
				Create();
			}
			
			if (!Cell->GetArray<FVoxelValue>())
			{
				Cell->CreateArray<FVoxelValue>();
			}
			if (!Cell->GetArray<FVoxelMaterial>())
			{
				Cell->CreateArray<FVoxelMaterial>();
			}
			bool bValuesAreSet;
			bool bMaterialsAreSet;
			Loader.CopyChunkToBuffers(Index, Cell->GetArray<FVoxelValue>(), Cell->GetArray<FVoxelMaterial>(), bValuesAreSet, bMaterialsAreSet);
			if (bValuesAreSet)
			{
				Cell->SetArrayAsDirty<FVoxelValue>();
			}
			if (bMaterialsAreSet)
			{
				Cell->SetArrayAsDirty<FVoxelMaterial>();
			}

			if (!bValuesAreSet || !bMaterialsAreSet)
			{
				FVoxelValue* Values = bValuesAreSet ? nullptr : Cell->GetArray<FVoxelValue>();
				FVoxelMaterial* Materials = bMaterialsAreSet ? nullptr : Cell->GetArray<FVoxelMaterial>();
				WorldGenerator->GetValuesAndMaterials(Values, Materials, FVoxelWorldGeneratorQueryZone(OctreeBounds, FIntVector(VOXEL_CELL_SIZE), 0), 0, *ItemHolder);
			}

			Index++;
			OutBoundsToUpdate.Add(GetBounds());
		}
	}
	else
	{
		if (GetBounds().IsInside(CurrentPosition))
		{
			if (IsLeaf())
			{
				CreateChildren();
			}
			for (auto& Child : GetChildren())
			{
				Child.Load(Index, Loader, OutBoundsToUpdate);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<EVoxelLockType LockType>
bool FVoxelDataOctree::TryLock(const FIntBox& Bounds, double TimeToTimeout, FVoxelLockedOctrees& OutIds, FString& InOutLockerName)
{
	if (!OctreeBounds.Intersect(Bounds))
	{
		TransactionsSection.Unlock();
		return true;
	}
	if (!Mutex.TryLockUntil<LockType>(TimeToTimeout))
	{
#if ENABLE_LOCKER_NAME
		{
			FScopeLock Lock(&LockerNameSection);
			InOutLockerName = LockerName.IsEmpty() ? "Timeout!" : (LockerName + "; Octree Bounds: " + OctreeBounds.ToString());
		}
#endif
		TransactionsSection.Unlock();
		return false;
	}

	if (IsLeaf())
	{
#if ENABLE_LOCKER_NAME
		{
			FScopeLock Lock(&LockerNameSection);
			LockerName = InOutLockerName;
		}
#endif
		OutIds.Add(Id);
		TransactionsSection.Unlock();
		return true;
	}
	else
	{
		Mutex.Unlock<LockType>();

		for (auto& Child : GetChildren())
		{
			Child.LockTransactions();
		}
		TransactionsSection.Unlock();

		for (int32 ChildIndex = 0; ChildIndex < 8 ; ChildIndex++)
		{
			if (!GetChild(ChildIndex).TryLock<LockType>(Bounds, TimeToTimeout, OutIds, InOutLockerName))
			{
				for (int32 Index = ChildIndex + 1; Index < 8 ; Index++)
				{
					GetChild(Index).TransactionsSection.Unlock();
				}
				return false;
			}
		}
		return true;
	}
}

template<EVoxelLockType LockType>
void FVoxelDataOctree::Unlock(FVoxelLockedOctrees& Ids)
{
	if (Ids.Num() > 0)
	{
		if (Ids.Last() == Id)
		{
			Ids.Pop(false);
#if ENABLE_LOCKER_NAME
			{
				FScopeLock Lock(&LockerNameSection);
				LockerName.Reset();
			}
#endif
			Mutex.Unlock<LockType>();
		}
		else if (IsIdChild(Ids.Last()))
		{
			for (int32 Index = 8 - 1; Index >= 0; Index--)
			{
				GetChild(Index).Unlock<LockType>(Ids);
			}
		}
	}
}

template bool FVoxelDataOctree::TryLock<EVoxelLockType::Read>(const FIntBox&, double, FVoxelLockedOctrees&, FString&);
template bool FVoxelDataOctree::TryLock<EVoxelLockType::ReadWrite>(const FIntBox&, double, FVoxelLockedOctrees&, FString&);

template void FVoxelDataOctree::Unlock<EVoxelLockType::Read>(FVoxelLockedOctrees&);
template void FVoxelDataOctree::Unlock<EVoxelLockType::ReadWrite>(FVoxelLockedOctrees&);

void FVoxelDataOctree::CreateChildren()
{
	TVoxelOctree::CreateChildren();
	
#if DO_THREADSAFE_CHECKS
	for (auto& Child : GetChildren())
	{
		Child.Parent = this;
	}
#endif

	const auto& AllItems = ItemHolder->GetAllItems();
	if (AllItems.Num() > 0)
	{
		for (auto& Child : GetChildren())
		{
			for (auto& Items : AllItems)
			{
				for (auto* Item : Items)
				{
					Child.AddItem(Item);
				}
			}
		}
	}
	ItemHolder.Reset();
}

void FVoxelDataOctree::DestroyChildren()
{
	TVoxelOctree::DestroyChildren();
	ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>(); // Always valid on a leaf
}

bool FVoxelDataOctree::Compact(uint32& NumDeleted)
{
	if (IsLeaf())
	{
		return (LOD > 0 || !IsCreated()) && ItemHolder->Num() == 0;
	}
	else
	{
		bool bCanCompact = true;
		for (auto& Child : GetChildren())
		{
			bCanCompact = Child.Compact(NumDeleted) && bCanCompact; // bCanCompact in second so that child is always compacted
		}
		if (bCanCompact)
		{
			DestroyChildren();
			NumDeleted += 8;
		}
		return bCanCompact;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::Create()
{
	ensureThreadSafe(IsLockedForWrite());

	check(LOD == 0 && !IsCreated());

	Cell = MakeUnique<FVoxelDataCell>();
	if (bEnableUndoRedo)
	{
		UndoRedoCell = MakeUnique<FVoxelDataCellUndoRedo>();
	}
}

void FVoxelDataOctree::Destroy()
{
	ensureThreadSafe(IsLockedForWrite());

	check(LOD == 0 && IsCreated());
	Cell.Reset();
	UndoRedoCell.Reset();
}

void FVoxelDataOctree::CreateArrayAndInitFromWorldGenerator(bool bInitValues, bool bInitMaterials)
{
	ensureThreadSafe(IsLockedForWrite());
	check(LOD == 0 && IsCreated());

	if (bInitValues || bInitMaterials)
	{
		if (bInitValues)
		{
			Cell->CreateArray<FVoxelValue>();
		}
		if (bInitMaterials)
		{
			Cell->CreateArray<FVoxelMaterial>();
		}
		auto* Values = bInitValues ? Cell->GetArray<FVoxelValue>() : nullptr;
		auto* Materials = bInitMaterials ? Cell->GetArray<FVoxelMaterial>() : nullptr;
		auto QueryZone = FVoxelWorldGeneratorQueryZone(OctreeBounds, FIntVector(VOXEL_CELL_SIZE), 0);
		if (WorldGenerator->HasCache2D())
		{
			float* Cache2DValues = Data->GetCache2DValues(FIntPoint(OctreeBounds.Min.X, OctreeBounds.Min.Y));
			WorldGenerator->GetValuesAndMaterialsCache2D(Values, Materials, Cache2DValues, QueryZone, *ItemHolder);
		}
		else
		{
			WorldGenerator->GetValuesAndMaterials(Values, Materials, QueryZone, 0, *ItemHolder);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::AddItem(FVoxelPlaceableItem* Item)
{	
	if (Item->Bounds.Intersect(OctreeBounds))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForWrite());

			ItemHolder->AddItem(Item);

			if (LOD == 0)
			{
				if (IsCreated())
				{
					if (IsCacheOnly())
					{
						ClearCache();
						ensure(!IsCreated());
					}
					else
					{
						Item->MergeWithOctree(this);
					}
				}
			}
			else if (ItemHolder->Num() > MAX_PLACEABLE_ITEMS_PER_OCTREE)
			{
				CreateChildren();
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.AddItem(Item);
			}
		}
	}
}

void FVoxelDataOctree::RemoveItem(FVoxelPlaceableItem* Item)
{
	if (Item->Bounds.Intersect(OctreeBounds))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForWrite());
			ItemHolder->RemoveItem(Item);

			if (LOD == 0 && IsCached())
			{
				ClearCache();
				ensure(!IsCreated());
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.RemoveItem(Item);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctree::SaveFrame(int32 HistoryPosition)
{	
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForWrite());
		if (LOD == 0 && IsCreated())
		{
			UndoRedoCell->SaveFrame(HistoryPosition);
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.SaveFrame(HistoryPosition);
		}
	}
}

void FVoxelDataOctree::Undo(int32 HistoryPosition, TArray<FIntBox>& OutBoundsToUpdate)
{
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForWrite());
		if (LOD == 0 && IsCreated())
		{
			if (UndoRedoCell->TryUndo(Cell.Get(), HistoryPosition))
			{
				OutBoundsToUpdate.Add(GetBounds());
			}
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.Undo(HistoryPosition, OutBoundsToUpdate);
		}
	}
}

void FVoxelDataOctree::Redo(int32 HistoryPosition, TArray<FIntBox>& OutBoundsToUpdate)
{	
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForWrite());
		if (LOD == 0 && IsCreated())
		{
			if (UndoRedoCell->TryRedo(Cell.Get(), HistoryPosition))
			{
				OutBoundsToUpdate.Add(GetBounds());
			}
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.Redo(HistoryPosition, OutBoundsToUpdate);
		}
	}
}

void FVoxelDataOctree::ClearFrames()
{	
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForWrite());
		if (LOD == 0 && IsCreated())
		{
			UndoRedoCell->ClearFrames();
		}
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			Child.ClearFrames();
		}
	}
}

bool FVoxelDataOctree::IsCurrentFrameEmpty() const
{
	if (IsLeaf())
	{
		ensureThreadSafe(IsLockedForRead());
		if (LOD == 0 && IsCreated())
		{
			return UndoRedoCell->IsCurrentFrameEmpty();
		}
		return true;
	}
	else
	{
		for (auto& Child : GetChildren())
		{
			if (!Child.IsCurrentFrameEmpty())
			{
				return false;
			}
		}
		return true;
	}
}