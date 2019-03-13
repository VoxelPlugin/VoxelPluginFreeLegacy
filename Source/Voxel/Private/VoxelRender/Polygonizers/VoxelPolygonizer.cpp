// Copyright 2019 Phyronnaz

#include "VoxelPolygonizer.h"
#include "VoxelData/VoxelData.h"
#include "Async/Async.h"
#include "VoxelDebug/VoxelDebugManager.h"

bool FVoxelPolygonizer::Create()
{
	const FIntBox ValuesBounds = GetValuesBounds();
	const FIntBox LockedBounds = GetLockedBounds();
	
	check(ValuesBounds.IsMultipleOf(Step));
	check(LockedBounds.IsMultipleOf(Step));

	Stats.StartStat("LockRead");
	Data->Lock<EVoxelLockType::Read>(LockedBounds, Octrees, FString::Printf(TEXT("Polygonizer LOD=%d"), LOD));

	Stats.StartStat("NeedToGenerateChunk");
	const bool bIsGenerating = LOD == 0 && Data->NeedToGenerateChunk(ChunkPosition);
	
	Stats.StartStat("IsEmpty");
	const bool bIsEmpty = Data->IsEmpty(ValuesBounds, LOD);
		
	Stats.StartStat("AreBoundsCached");
	const bool bNeedsCache = LOD == 0 && !bIsEmpty && Data->bCacheLOD0Chunks && !Data->AreBoundsCached(LockedBounds);
	
	Stats.SetIsEmpty(bIsEmpty);
	Stats.StartStat("IsEmptyDebug");
	{
		FIntBox BoundsCopy(ValuesBounds.Min, ValuesBounds.Max - FIntVector(Step));
		AsyncTask(ENamedThreads::GameThread, [WeakDebug = TWeakPtr<FVoxelDebugManager, ESPMode::ThreadSafe>(DebugManager), BoundsCopy, bIsEmpty]
		{
			auto Debug = WeakDebug.Pin();
			if (Debug.IsValid())
			{
				Debug->ReportChunkEmptyState(BoundsCopy, bIsEmpty);
			}
		});
	}

	if (bNeedsCache || bIsGenerating)
	{
		Stats.StartStat("UnlockReadBeforeCache");
		Data->Unlock<EVoxelLockType::Read>(Octrees);

		Stats.StartStat("LockReadWrite");
		Data->Lock<EVoxelLockType::ReadWrite>(LockedBounds, Octrees, FString::Printf(TEXT("Polygonizer LOD=%d Cache"), LOD));
		
		if (bNeedsCache)
		{
			Stats.StartStat("Cache");
			Data->CacheBounds(LockedBounds, false, true, false); // LockedBounds and not ValuesBounds as we want to cache all accesses
		}
		if (bIsGenerating)
		{
			Stats.StartStat("PreGenerationDelegate");
			Data->LockDelegatesForRead();
			Data->PreGenerationDelegate.Broadcast(*Data, FVoxelUtilities::GetBoundsFromPositionAndLOD<CHUNK_SIZE>(ChunkPosition, LOD));
			Data->UnlockDelegatesForRead();
		}

		Stats.StartStat("UnlockReadWrite");
		Data->Unlock<EVoxelLockType::ReadWrite>(Octrees);

		Stats.StartStat("LockReadAfterCache");
		Data->Lock<EVoxelLockType::Read>(LockedBounds, Octrees, FString::Printf(TEXT("Polygonizer LOD=%d"), LOD));
	}

	if (bIsEmpty)
	{
		Stats.StartStat("UnlockRead");
		Data->Unlock<EVoxelLockType::Read>(Octrees);
	}
	else
	{
		if (!CreateChunk())
		{
			return false;
		}
	}

	if (bIsGenerating)
	{
		Stats.StartStat("PostGenerationDelegate");
		Data->LockDelegatesForRead();
		Data->PostGenerationDelegate.Broadcast(*Data, FVoxelUtilities::GetBoundsFromPositionAndLOD<CHUNK_SIZE>(ChunkPosition, LOD), Chunk);
		Data->UnlockDelegatesForRead();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelTransitionsPolygonizer::Create()
{
	if (!TransitionsMask)
	{
		return true;
	}

	const FIntBox Bounds = GetBounds();
	check(Bounds.IsMultipleOf(Step));

	Stats.StartStat("LockRead");
	Data->Lock<EVoxelLockType::Read>(Bounds, Octrees, FString::Printf(TEXT("TransitionsPolygonizer LOD=%d"), LOD));

	Stats.StartStat("IsEmpty");
	bool bIsEmpty = Data->IsEmpty(Bounds, LOD);
	Stats.SetIsEmpty(bIsEmpty);

	if (bIsEmpty)
	{
		Stats.StartStat("UnlockRead");
		Data->Unlock<EVoxelLockType::Read>(Octrees);
		return true;
	}

	return CreateTransitions();
}
