// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelData.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelData/VoxelDataOctree.h"
#include "VoxelSave.h"
#include "VoxelDiff.h"
#include "VoxelWorldGenerator.h"
#include "Algo/Reverse.h"
#include "Misc/ScopeLock.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelData::LoadFromDiffQueues"), STAT_VoxelData_LoadFromDiffQueues, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelData::LoadFromDiffQueues::BeginSet"), STAT_VoxelData_LoadFromDiffQueues_BeginSet, STATGROUP_Voxel);

void FVoxelData::BeginSet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees, FString Name)
{
	MainLock.lock_shared();

	OutOctrees.Empty();

	Octree->LockTransactions();
	Octree->BeginSet(Box, OutOctrees, 1e9, Name);

	Algo::Reverse(OutOctrees);
}

bool FVoxelData::TryBeginSet(const FIntBox& Box, int MicroSeconds, TArray<FVoxelId>& OutOctrees, FString& InOutName)
{
	MainLock.lock_shared();

	OutOctrees.Empty();

	Octree->LockTransactions();

	bool bSuccess = Octree->BeginSet(Box, OutOctrees, MicroSeconds, InOutName);

	Algo::Reverse(OutOctrees);

	if (!bSuccess)
	{
		EndSet(OutOctrees);
	}

	return bSuccess;
}

void FVoxelData::EndSet(TArray<FVoxelId>& LockedOctrees)
{
	Octree->EndSet(LockedOctrees);
	check(LockedOctrees.Num() == 0);

	MainLock.unlock_shared();
}

void FVoxelData::BeginGet(const FIntBox& Box, TArray<FVoxelId>& OutOctrees, FString Name)
{
	MainLock.lock_shared();

	OutOctrees.Empty();

	Octree->LockTransactions();
	Octree->BeginGet(Box, OutOctrees, 1e9, Name);

	Algo::Reverse(OutOctrees);
}

bool FVoxelData::TryBeginGet(const FIntBox& Box, int MicroSeconds, TArray<FVoxelId>& OutOctrees, FString& InOutName)
{
	MainLock.lock_shared();

	OutOctrees.Empty();

	Octree->LockTransactions();

	bool bSuccess = Octree->BeginGet(Box, OutOctrees, MicroSeconds, InOutName);

	Algo::Reverse(OutOctrees);

	if (!bSuccess)
	{
		EndGet(OutOctrees);
	}

	return bSuccess;
}

void FVoxelData::EndGet(TArray<FVoxelId>& LockedOctrees)
{
	Octree->EndGet(LockedOctrees);
	check(LockedOctrees.Num() == 0);
	
	MainLock.unlock_shared();
}

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
		FVoxelScopeGetLock Lock(this, FIntBox::Infinite, "CacheMostUsedChunks FindChunks");
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
			FVoxelScopeSetLock Lock(this, Chunk->GetBounds(), "CacheMostUsedChunks CacheOctrees");
			check(Chunk->LOD == 0);
			if (!Chunk->IsCreated()) // Might have been created since FindChunks
			{
				NumChunksCached++;
				TotalNumCachedChunks++;
				Chunk->Cache(false, false);
			}
		}
		else
		{
			TotalNumCachedChunks++;
		}
	}

	for (auto& OctreeToSubdivide : OctreesToSubdivide)
	{
		FVoxelScopeSetLock Lock(this, OctreeToSubdivide->GetBounds(), "CacheMostUsedChunks SubdivideOctrees");
		if (OctreeToSubdivide->IsLeaf())
		{
			NumChunksSubdivided++;
			OctreeToSubdivide->CreateChildren();
		}
	}
}

void FVoxelData::Compact(uint32& NumDeleted)
{
	MainLock.lock();

	NumDeleted = 0;
	Octree->Compact(NumDeleted);

	MainLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelData::GetSave(FVoxelUncompressedWorldSave& OutSave)
{
	FVoxelScopeGetLock Lock(this, FIntBox::Infinite, "GetSave");

	OutSave.Init(Depth);
	Octree->Save(OutSave);
	OutSave.Save();
}

void FVoxelData::LoadFromSave(const FVoxelUncompressedWorldSave& Save, TArray<FIntBox>& OutBoundsToUpdate)
{
	FVoxelScopeSetLock Lock(this, FIntBox::Infinite, "LoadFromSave");

	Octree->GetLeavesBounds(OutBoundsToUpdate, 0);
	Octree->ClearData();
	if (bEnableUndoRedo)
	{
		HistoryPosition = 0;
		MaxHistoryPosition = 0;
	}

	int Index = 0;
	Octree->Load(Index, Save, OutBoundsToUpdate);

	check(Index == Save.NumChunks() || Save.GetDepth() > Depth);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

