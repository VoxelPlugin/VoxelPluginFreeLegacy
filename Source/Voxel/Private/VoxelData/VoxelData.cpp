// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelWorldGenerator.h"
#include "VoxelData/VoxelSaveUtilities.h"

#include "Algo/Reverse.h"
#include "Misc/ScopeLock.h"
#include "Async/Async.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelData::LoadFromDiffQueues"), STAT_VoxelData_LoadFromDiffQueues, STATGROUP_Voxel);

template<EVoxelLockType LockType>
bool FVoxelData::TryLock(const FIntBox& Bounds, float TimeoutInSeconds, FVoxelLockedOctrees& OutOctrees, FString& InOutName)
{
	ensure(!InOutName.IsEmpty());
	InOutName = (LockType == EVoxelLockType::Read ? "Read Lock: " : "ReadWrite Lock: ") + InOutName;
	OutOctrees.Reset();

	MainLock.Lock<EVoxelLockType::Read>();

	double StartTime = FPlatformTime::Seconds();

	Octree->LockTransactions();

	bool bSuccess = Octree->TryLock<LockType>(Bounds, StartTime + TimeoutInSeconds, OutOctrees, InOutName);

	double Duration = FPlatformTime::Seconds() - StartTime;
	if (Duration > TimeoutInSeconds)
	{
		UE_LOG(LogVoxel, Warning, TEXT("TryLock took longer than timeout: %fs instead of %fs. Success: %s"), Duration, TimeoutInSeconds, bSuccess ? TEXT("true") : TEXT("false"));
	}

	if (!bSuccess)
	{
#if 0 // Way faster, but there might be race conditions if we relock it right away on the same thread
		if (OutOctrees.Num() == 0)
		{
			Unlock<LockType>(OutOctrees);
		}
		else
		{
			AsyncTask(ENamedThreads::AnyThread, [Octrees = OutOctrees, WeakData = TWeakPtr<FVoxelData, ESPMode::ThreadSafe>(AsShared())]()
			{
				auto Data = WeakData.Pin();
				if (Data.IsValid())
				{
					double StartTime = FPlatformTime::Seconds();
					auto NotConstOctrees = Octrees;
					Data->Unlock<LockType>(NotConstOctrees);
					double Duration = FPlatformTime::Seconds() - StartTime;
					AsyncTask(ENamedThreads::GameThread, [Duration]()
					{
						UE_LOG(LogVoxel, Log, TEXT("Unlocking failed TryLock took %fs"), Duration);
					});
				}
			});
		}
		OutOctrees.Reset();
#endif
		Unlock<LockType>(OutOctrees);
	}

	return bSuccess;
}

template<EVoxelLockType LockType>
void FVoxelData::Unlock(FVoxelLockedOctrees& LockedOctrees)
{
	Octree->Unlock<LockType>(LockedOctrees);
	if (!ensureMsgf(LockedOctrees.Num() == 0, TEXT("Unlock failed! %d octrees remaining"), LockedOctrees.Num()))
	{
		LockedOctrees.Reset();
	}

	MainLock.Unlock<EVoxelLockType::Read>();
}

template VOXEL_API bool FVoxelData::TryLock<EVoxelLockType::Read>(const FIntBox&, float, FVoxelLockedOctrees&, FString&);
template VOXEL_API bool FVoxelData::TryLock<EVoxelLockType::ReadWrite>(const FIntBox&, float, FVoxelLockedOctrees&, FString&);

template VOXEL_API void FVoxelData::Unlock<EVoxelLockType::Read>(FVoxelLockedOctrees&);
template VOXEL_API void FVoxelData::Unlock<EVoxelLockType::ReadWrite>(FVoxelLockedOctrees&);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::ClearData()
{
	Octree->ClearData();
	HistoryPosition = 0;
	MaxHistoryPosition = 0;
	bIsDirty = true;
	
	FScopeLock Lock(&ItemsSection);
	FreeItems.Empty();
	Items.Empty();
	ItemFrame = MakeUnique<FItemFrame>();
	ItemUndoFrames.Empty();
	ItemRedoFrames.Empty();
}

void FVoxelData::GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int32 QueryLOD) const
{
	auto& OctreeBounds = Octree->GetBounds();

	check(OctreeBounds.IsMultipleOf(QueryZone.Step));

	Octree->GetValuesAndMaterials(Values, Materials, QueryZone, QueryLOD);

	if (!OctreeBounds.Contains(QueryZone.Bounds))
	{
		TArray<FIntBox> BoundsOutsideWorld;
		FIntBox::GetRemainingBoxes(QueryZone.Bounds, OctreeBounds, BoundsOutsideWorld);

		for (auto& LocalBounds : BoundsOutsideWorld)
		{
			check(LocalBounds.IsMultipleOf(QueryZone.Step));
			WorldGenerator->GetValuesAndMaterials(Values, Materials, QueryZone.ShrinkTo(LocalBounds), QueryLOD, FVoxelPlaceableItemHolder());
		}
	}
}

void FVoxelData::CacheMostUsedChunks(
	uint32 Threshold,
	uint32 MaxCacheSize,
	uint32& NumChunksSubdivided,
	uint32& NumChunksCached, 
	uint32& NumRemovedFromCache,
	uint32& TotalNumCachedChunks)
{
	FScopeLock CacheLock(&CacheSection);

	CacheTime++;
	NumChunksSubdivided = 0;
	NumChunksCached = 0;
	NumRemovedFromCache = 0;

	TArray<FVoxelDataOctree::CacheElement> OctreesToCacheAndCachedOctrees;
	TArray<FVoxelDataOctree*> OctreesToSubdivide;
	{
		FVoxelReadScopeLock Lock(this, FIntBox::Infinite, "CacheMostUsedChunks FindChunks");
		Octree->GetOctreesToCacheAndExistingCachedOctrees(CacheTime, Threshold, OctreesToCacheAndCachedOctrees, OctreesToSubdivide);
	}

	if ((uint32)OctreesToCacheAndCachedOctrees.Num() > MaxCacheSize)
	{
		OctreesToCacheAndCachedOctrees.Sort([](auto& A, auto& B) { return A.Priority > B.Priority; }); // Highest first
		while ((uint32)OctreesToCacheAndCachedOctrees.Num() > MaxCacheSize)
		{
			auto CacheElement = OctreesToCacheAndCachedOctrees.Pop(false);
			if (CacheElement.bIsCached)
			{
				FVoxelReadWriteScopeLock Lock(this, CacheElement.Octree->GetBounds(), "CacheMostUsedChunks ClearCache");
				CacheElement.Octree->ClearCache();
				NumRemovedFromCache++;
			}
		}
	}
	
	for (auto& CacheElement : OctreesToCacheAndCachedOctrees)
	{
		if (!CacheElement.bIsCached)
		{
			auto* Chunk = CacheElement.Octree;
			FVoxelReadWriteScopeLock Lock(this, Chunk->GetBounds(), "CacheMostUsedChunks CacheOctrees");
			check(Chunk->LOD == 0);
			if (!Chunk->IsCreated()) // Might have been created since FindChunks
			{
				NumChunksCached++;
				TotalNumCachedChunks++;
				Chunk->Cache(false, true, false);
			}
		}
		else
		{
			TotalNumCachedChunks++;
		}
	}

	for (auto& OctreeToSubdivide : OctreesToSubdivide)
	{
		FVoxelReadWriteScopeLock Lock(this, OctreeToSubdivide->GetBounds(), "CacheMostUsedChunks SubdivideOctrees");
		if (OctreeToSubdivide->IsLeaf())
		{
			NumChunksSubdivided++;
			OctreeToSubdivide->CreateChildren();
		}
	}
}

void FVoxelData::Compact(uint32& NumDeleted)
{
	NumDeleted = 0;
	
	// Can't allow compact & cache to run at the same time as cache have ref to octrees
	FScopeLock CacheLock(&CacheSection);
	MainLock.Lock<EVoxelLockType::ReadWrite>();
	Octree->Compact(NumDeleted);
	MainLock.Unlock<EVoxelLockType::ReadWrite>();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetSave(FVoxelUncompressedWorldSave& OutSave)
{
	FVoxelReadScopeLock Lock(this, FIntBox::Infinite, "GetSave");

	FVoxelSaveBuilder Builder(Depth);

	Octree->Save(Builder);

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

void FVoxelData::LoadFromSave(const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate)
{
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "LoadFromSave");

	Octree->GetLeavesBounds(OutBoundsToUpdate, 0);
	
	ClearData();

	FVoxelSaveLoader Loader(Save);

	int32 Index = 0;
	Octree->Load(Index, Loader, OutBoundsToUpdate);
	
	for (auto& Item : Loader.GetPlaceableItems())
	{
		AddItem(Item.ToSharedRef(), false);
	}

	check(Index == Loader.NumChunks() || Save.GetDepth() > Depth);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_UNDO_REDO_IMPL(r) check(IsInGameThread()); if(!ensure(bEnableUndoRedo)) { return r; }
#define NOARG
#define CHECK_UNDO_REDO() CHECK_UNDO_REDO_IMPL(NOARG)
#define CHECK_UNDO_REDO_BOOL() CHECK_UNDO_REDO_IMPL(false)

void FVoxelData::Undo(TArray<FIntBox>& OutBoundsToUpdate)
{
	CHECK_UNDO_REDO();

	if (HistoryPosition > 0)
	{
		MarkAsDirty();
		HistoryPosition--;
		
		FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "Undo");
		{
			FScopeLock ItemLock(&ItemsSection);
			check(ItemFrame->IsEmpty());
			if (ItemUndoFrames.Num() > 0 && ItemUndoFrames.Last()->HistoryPosition == HistoryPosition)
			{
				auto UndoFrame = ItemUndoFrames.Pop(false);

				for (auto& Item : UndoFrame->AddedItems)
				{
					RemoveItem(Item.Get(), true);
					OutBoundsToUpdate.Add(Item->Bounds);
				}
				for (auto& Item : UndoFrame->RemovedItems)
				{
					AddItem(Item.ToSharedRef(), true);
					OutBoundsToUpdate.Add(Item->Bounds);
				}

				UndoFrame->HistoryPosition = HistoryPosition + 1;
				ItemRedoFrames.Add(MoveTemp(UndoFrame));
				ItemFrame = MakeUnique<FItemFrame>();
			}
		}
		Octree->Undo(HistoryPosition, OutBoundsToUpdate);
	}
}

void FVoxelData::Redo(TArray<FIntBox>& OutBoundsToUpdate)
{
	CHECK_UNDO_REDO();

	if (HistoryPosition < MaxHistoryPosition)
	{
		MarkAsDirty();
		HistoryPosition++;

		FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "Redo");
		{
			FScopeLock ItemLock(&ItemsSection);
			check(ItemFrame->IsEmpty());
			if (ItemRedoFrames.Num() > 0 && ItemRedoFrames.Last()->HistoryPosition == HistoryPosition)
			{
				auto RedoFrame = ItemRedoFrames.Pop(false);

				for (auto& Item : RedoFrame->AddedItems)
				{
					AddItem(Item.ToSharedRef(), true);
					OutBoundsToUpdate.Add(Item->Bounds);
				}
				for (auto& Item : RedoFrame->RemovedItems)
				{
					RemoveItem(Item.Get(), true);
					OutBoundsToUpdate.Add(Item->Bounds);
				}

				RedoFrame->HistoryPosition = HistoryPosition - 1;
				ItemUndoFrames.Add(MoveTemp(RedoFrame));
				ItemFrame = MakeUnique<FItemFrame>();
			}
		}
		Octree->Redo(HistoryPosition, OutBoundsToUpdate);
	}
}

void FVoxelData::ClearFrames()
{
	CHECK_UNDO_REDO();

	HistoryPosition = 0;
	MaxHistoryPosition = 0;

	{
		FScopeLock Lock(&ItemsSection);
		ItemFrame = MakeUnique<FItemFrame>();
		ItemUndoFrames.Empty();
		ItemRedoFrames.Empty();
	}

	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "ClearFrames");
	Octree->ClearFrames();
}

void FVoxelData::SaveFrame()
{
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
	{
		FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "SaveFrame");
		Octree->SaveFrame(HistoryPosition);
	}

	HistoryPosition++;
	MaxHistoryPosition = HistoryPosition;
}

bool FVoxelData::IsCurrentFrameEmpty()
{
	CHECK_UNDO_REDO_BOOL();

	{
		FScopeLock Lock(&ItemsSection);
		if (!ItemFrame->IsEmpty())
		{
			return false;
		}
	}

	FVoxelReadScopeLock Lock(this, FIntBox::Infinite, "IsCurrentFrameEmpty");
	return Octree->IsCurrentFrameEmpty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::AddItem(const TSharedRef<FVoxelPlaceableItem>& Item, bool bRecordInHistory)
{
	Octree->AddItem(&Item.Get());

	FScopeLock Lock(&ItemsSection);
	if (FreeItems.Num() == 0)
	{
		Item->ItemIndex = Items.Add(Item);
	}
	else
	{
		int32 Index = FreeItems.Pop();
		check(!Items[Index].IsValid());
		Item->ItemIndex = Index;
		Items[Index] = Item;
	}
	if (bEnableUndoRedo && bRecordInHistory)
	{
		ItemFrame->AddedItems.Add(Item);
	}
}

void FVoxelData::RemoveItem(FVoxelPlaceableItem* Item, bool bRecordInHistory)
{
	check(Item);

	if (!ensureMsgf(Items.IsValidIndex(Item->ItemIndex), TEXT("Invalid item: %s"), *Item->GetDescription()) ||
		!ensureMsgf(Items[Item->ItemIndex].IsValid(), TEXT("Item already removed: %s"), *Item->GetDescription()))
	{
		return;
	}

	Octree->RemoveItem(Item);
	
	FScopeLock Lock(&ItemsSection);
	auto& ItemPtr = Items[Item->ItemIndex];
	if (bEnableUndoRedo && bRecordInHistory)
	{
		ItemFrame->RemovedItems.Add(ItemPtr);
	}
	ItemPtr.Reset();
	FreeItems.Add(Item->ItemIndex);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

float* FVoxelData::GetCache2DValues(const FIntPoint& Position)
{
	Cache2DLock.lock_shared();
	TUniquePtr<FCache2DValues>* Values = Cache2DValues.Find(Position);
	Cache2DLock.unlock_shared();
	if (Values)
	{
		return Values->Get()->Values;
	}
	else
	{
		TUniquePtr<FCache2DValues> NewValues = MakeUnique<FCache2DValues>();
		WorldGenerator->Cache2D(NewValues->Values, FVoxelWorldGeneratorQueryZone2D(Position, Position + FIntPoint(VOXEL_CELL_SIZE, VOXEL_CELL_SIZE)));
		float* ReturnValues = NewValues->Values;

		Cache2DLock.lock();
		if (TUniquePtr<FCache2DValues>* ExistingValues = Cache2DValues.Find(Position))
		{
			UE_LOG(LogVoxel, Warning, TEXT("Position (%d, %d) was cached twice, dropping one"), Position.X, Position.Y);
			ReturnValues = ExistingValues->Get()->Values;
		}
		else
		{
			Cache2DValues.Add(Position, MoveTemp(NewValues));
			Cache2DValues.Compact();
		}
		Cache2DLock.unlock();

		return ReturnValues;
	}
}

bool FVoxelData::NeedToGenerateChunk(const FIntVector& Position)
{
	FScopeLock Lock(&GeneratedChunksSection);

	const FIntVector Index = Position / CHUNK_SIZE;
	if (GeneratedChunks.Contains(Index))
	{
		return false;
	}
	else
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelGeneratedChunksSetMemory, GeneratedChunks.GetAllocatedSize());
		GeneratedChunks.Add(Index);
		GeneratedChunks.Compact();
		INC_MEMORY_STAT_BY(STAT_VoxelGeneratedChunksSetMemory, GeneratedChunks.GetAllocatedSize());

		return true;
	}
}
