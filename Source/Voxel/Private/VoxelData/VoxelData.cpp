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

template bool FVoxelData::TryLock<EVoxelLockType::Read>(const FIntBox&, float, FVoxelLockedOctrees&, FString&);
template bool FVoxelData::TryLock<EVoxelLockType::ReadWrite>(const FIntBox&, float, FVoxelLockedOctrees&, FString&);

template void FVoxelData::Unlock<EVoxelLockType::Read>(FVoxelLockedOctrees&);
template void FVoxelData::Unlock<EVoxelLockType::ReadWrite>(FVoxelLockedOctrees&);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD) const
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
	FScopeLock CacheLock(&CacheTimeSection);

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
	Builder.Save(OutSave);
}

void FVoxelData::LoadFromSave(const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate)
{
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "LoadFromSave");

	Octree->GetLeavesBounds(OutBoundsToUpdate, 0);
	Octree->ClearData();
	if (bEnableUndoRedo)
	{
		HistoryPosition = 0;
		MaxHistoryPosition = 0;
	}

	FVoxelSaveLoader Loader(Save);

	int Index = 0;
	Octree->Load(Index, Loader, OutBoundsToUpdate);

	check(Index == Loader.NumChunks() || Save.GetDepth() > Depth);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::Undo(TArray<FIntBox>& OutBoundsToUpdate)
{
	check(IsInGameThread());
	ensure(bEnableUndoRedo);
	MarkAsDirty();
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "Undo");
	if (HistoryPosition > 0)
	{
		HistoryPosition--;
		Octree->Undo(HistoryPosition, OutBoundsToUpdate);
	}
}

void FVoxelData::Redo(TArray<FIntBox>& OutBoundsToUpdate)
{
	check(IsInGameThread());
	ensure(bEnableUndoRedo);
	MarkAsDirty();
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "Redo");
	if (HistoryPosition < MaxHistoryPosition)
	{
		HistoryPosition++;
		Octree->Redo(HistoryPosition, OutBoundsToUpdate);
	}
}

void FVoxelData::ClearFrames()
{
	check(IsInGameThread());
	ensure(bEnableUndoRedo);
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "ClearFrames");
	Octree->ClearFrames();
	HistoryPosition = 0;
	MaxHistoryPosition = 0;
}

void FVoxelData::SaveFrame()
{
	check(IsInGameThread());
	ensure(bEnableUndoRedo);
	MarkAsDirty();
	FVoxelReadWriteScopeLock Lock(this, FIntBox::Infinite, "SaveFrame");
	Octree->SaveFrame(HistoryPosition);
	HistoryPosition++;
	MaxHistoryPosition = HistoryPosition;
}

bool FVoxelData::IsCurrentFrameEmpty()
{
	check(IsInGameThread());
	ensure(bEnableUndoRedo);
	FVoxelReadScopeLock Lock(this, FIntBox::Infinite, "IsCurrentFrameEmpty");
	return Octree->IsCurrentFrameEmpty();
}
