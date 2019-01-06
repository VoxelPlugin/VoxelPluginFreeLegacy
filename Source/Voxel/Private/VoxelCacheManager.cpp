// Copyright 2019 Phyronnaz

#include "VoxelCacheManager.h"
#include "VoxelData/VoxelData.h"
#include "VoxelGlobals.h"
#include "VoxelWorld.h"
#include "Engine/Engine.h"
#include "VoxelLogStatDefinitions.h"

const FColor FVoxelCacheManager::CreatedDirty = FColorList::Red;
const FColor FVoxelCacheManager::CreatedManualCached = FColorList::Pink;
const FColor FVoxelCacheManager::CreatedAutoCached = FColorList::Orange;
const FColor FVoxelCacheManager::Empty = FColorList::White;

///////////////////////////////////////////////////////////////////////////////

void FVoxelAsyncCacheWork::DoWork()
{
	Progress.Reset();

	for (auto& Bounds : BoundsToCache)
	{
		TArray<FVoxelId> Ids;
		Data->BeginSet(Bounds, Ids, "AsyncCacheWork");
		check(Ids.Num() == 1);

		auto* Leaf = Data->GetOctree()->CreateLeafAt(Bounds.Min);
		Leaf->Cache(true, bCheckIfChunksAreEmpty);

		CachedOctrees.Add(Bounds, Leaf);

		Data->EndSet(Ids);

		Progress.Increment();
		if (IsCanceled())
		{
			break;
		}
	}
}

void FVoxelAsyncClearCacheWork::DoWork()
{	
	for (auto& Octree : OctreesToClear)
	{
		TArray<FVoxelId> Ids;
		Data->BeginSet(Octree->GetBounds(), Ids, "AsyncClearCacheWork");
		check(Ids.Num() == 1);

		if (Octree->IsManuallyCached()) // If we cached it (ie if it wasn't already cached when when tried to cache it)
		{
			Octree->ClearManualCache();
		}

		Data->EndSet(Ids);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCacheManager::~FVoxelCacheManager()
{
	if (CacheWork.IsValid())
	{
		if (!CacheWork->IsDone() && !World->GetPool()->MeshPool->RetractQueuedWork(CacheWork.Get()))
		{
			CacheWork->CancelAndAutodelete();
			CacheWork.Release();
		}
	}
}

void FVoxelCacheManager::Cache(const TArray<FIntBox>& BoundsToCache, bool bCheckIfChunksAreEmpty)
{
	UpdateFromWork();

	if (CacheWork.IsValid())
	{
		UE_LOG(LogVoxel, Warning, TEXT("Voxel Cache: Cache called but work is still pending, canceling"));
		return;
	}

	TArray<FIntBox> NewBoundsToCache;
	for (auto& Bounds : BoundsToCache)
	{
		TArray<FIntBox> Childs;
		Bounds.Overlap(World->GetBounds()).Subdivide(VOXEL_CELL_SIZE, Childs);
		for (auto& Child : Childs)
		{
			if (!CachedOctrees.Contains(Child))
			{
				CachedOctrees.Add(Child);
				NewBoundsToCache.Add(Child);
			}
		}
	}

	CacheWork = MakeUnique<FVoxelAsyncCacheWork>(World->GetDataSharedPtr(), NewBoundsToCache, bCheckIfChunksAreEmpty);
	World->GetPool()->MeshPool->AddQueuedWork(CacheWork.Get());
}

void FVoxelCacheManager::ClearCache(const TArray<FIntBox>& BoundsToKeepCached)
{
	UpdateFromWork();

	TArray<FVoxelDataOctree*> OctreesToRemove;

	for (auto It = CachedOctrees.CreateIterator(); It; ++It)
	{
		auto& Bounds = It.Key();
		auto& Octree = It.Value();

		bool bKeep = false;
		for (auto& BoundToKeep : BoundsToKeepCached)
		{
			if (Bounds.Intersect(BoundToKeep))
			{
				bKeep = true;
				break;
			}
		}

		if (!bKeep)
		{
			if (Octree)
			{
				OctreesToRemove.Add(Octree);
			}
			It.RemoveCurrent();
		}
	}
	
	// Will autodelete
	auto* ClearCacheWork = new FVoxelAsyncClearCacheWork(World->GetDataSharedPtr(), OctreesToRemove);
	World->GetPool()->MeshPool->AddQueuedWork(ClearCacheWork);
}

void FVoxelCacheManager::DebugCache(bool bHideEmpty, float DeltaTime, float Thickness)
{
	UpdateFromWork();

	GEngine->AddOnScreenDebugMessage((uint64)this + 3, 1, CreatedAutoCached, TEXT("Not dirty chunks automatically cached"));
	GEngine->AddOnScreenDebugMessage((uint64)this + 2, 1, CreatedManualCached, TEXT("Not dirty chunks manually cached"));
	GEngine->AddOnScreenDebugMessage((uint64)this + 1, 1, Empty, TEXT("Empty chunks"));
	GEngine->AddOnScreenDebugMessage((uint64)this + 0, 1, CreatedDirty, TEXT("Dirty chunks"));

	auto* Data = World->GetData();
	FVoxelScopeGetLock Lock(Data, FIntBox::Infinite, "DebugCache");
	TArray<FVoxelDataOctree*> Octrees;
	Data->GetOctree()->GetLeaves(Octrees, 0);

	for (auto& Octree : Octrees)
	{
		FIntBox Bounds = Octree->GetBounds();

		auto** CachedOctree = CachedOctrees.Find(Bounds);
		check(!CachedOctree || !*CachedOctree || *CachedOctree == Octree);

		FColor Color;
		if (Octree->IsCreated())
		{
			if (Octree->IsCacheOnly())
			{
				if (Octree->IsManuallyCached())
				{
					check(CachedOctree);
					Color = CreatedManualCached;
				}
				else
				{
					Color = CreatedAutoCached;
				}
			}
			else
			{
				Color = CreatedDirty;
			}
		}
		else
		{
			if (Octree->IsEmpty())
			{					
				if (bHideEmpty)
				{
					continue;
				}
				Color = Empty;
			}
			else
			{
				// Just a basic LOD 0 leaf, nothing to print here
				continue;
			}
		}
		World->DrawDebugIntBox(Bounds, 2 * DeltaTime, Thickness, Color, Thickness);
	}
}

void FVoxelCacheManager::UpdateFromWork()
{
	if (CacheWork.IsValid() && CacheWork->IsDone())
	{
		for (auto& It : CacheWork->CachedOctrees)
		{
			auto Bounds = It.Key;
			auto* Octree = It.Value;
			check(!CachedOctrees[Bounds]);
			CachedOctrees[Bounds] = Octree;
		}
		CacheWork.Reset();
	}
}

FString FVoxelCacheManager::GetProgress() const
{
	if (!CacheWork.IsValid())
	{
		return "";
	}
	return FString::Printf(TEXT("%d/%d"), CacheWork->Progress.GetValue(), CacheWork->Total);
}