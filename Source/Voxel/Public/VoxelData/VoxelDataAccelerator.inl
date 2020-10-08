// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelData.inl"
#include "VoxelData/VoxelDataUtilities.h"

#if VOXEL_DATA_ACCELERATOR_STATS
#define ACCELERATOR_STAT(X) X
#else
#define ACCELERATOR_STAT(X)
#endif

template<typename TData>
TVoxelDataAccelerator<TData>::TVoxelDataAccelerator(TData& Data, int32 CacheSize)
	: Data(Data)
	, Bounds(FVoxelIntBox::Infinite)
	, CacheSize(CacheSize)
	, bUseAcceleratorMap(false)
{
	CacheEntries.Reserve(CacheSize);
}

template<typename TData>
TVoxelDataAccelerator<TData>::TVoxelDataAccelerator(TData& Data, const FVoxelIntBox& Bounds, const TVoxelDataAccelerator<TData>* MapSource, int32 CacheSize)
	: Data(Data)
	, Bounds(Bounds)
	, CacheSize(CacheSize)
	, bUseAcceleratorMap(FVoxelDataAcceleratorParameters::GetUseAcceleratorMap())
	, AcceleratorMap(MapSource ? MapSource->AcceleratorMap : GetAcceleratorMap(Data, Bounds))
{
	check(!MapSource || bIsConst);
	ensure(!MapSource || bUseAcceleratorMap == MapSource->bUseAcceleratorMap);
	CacheEntries.Reserve(CacheSize);
}

template<typename TData>
TVoxelDataAccelerator<TData>::~TVoxelDataAccelerator()
{
#if VOXEL_DATA_ACCELERATOR_STATS
	if (FVoxelDataAcceleratorParameters::GetShowStats() && (NumGet > 0 || NumSet > 0))
	{
		LOG_VOXEL(
			Log,
			TEXT("DataAccelerator: %6u reads; %6u writes; %6u/%6u top cache miss (%3.2f%% hits); %6u/%6u other cache miss (%3.2f%% hits); %6u/%6u map miss (%3.2f%% hits); %6u out of world"),
			NumGet,
			NumSet,
			NumCacheTopMiss,
			NumCacheTopAccess,
			NumCacheTopAccess > 0 ? 100 * double(NumCacheTopAccess - NumCacheTopMiss) / NumCacheTopAccess : 0,
			NumCacheAllMiss,
			NumCacheAllAccess,
			NumCacheAllAccess > 0 ? 100 * double(NumCacheAllAccess - NumCacheAllMiss) / NumCacheAllAccess : 0,
			NumMapMiss,
			NumMapAccess,
			NumMapAccess > 0 ? 100 * double(NumMapAccess - NumMapMiss) / NumMapAccess : 0,
			NumOutOfWorld);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<typename TData>
template<typename T>
FORCEINLINE T TVoxelDataAccelerator<TData>::GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
{
	// Clamp to world, to avoid un-editable border
	Data.ClampToWorld(X, Y, Z);

	return GetImpl(X, Y, Z,
	               [&](const FVoxelDataOctreeBase& Octree)
	               {
		               return Octree.GetCustomOutput<T>(*Data.Generator, DefaultValue, Name, X, Y, Z, LOD);
	               });
}

template<typename TData>
FORCEINLINE v_flt TVoxelDataAccelerator<TData>::GetFloatValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, bool* bIsGeneratorValue) const
{
	// Clamp to world, to avoid un-editable border
	Data.ClampToWorld(X, Y, Z);

	return GetImpl(int32(X), int32(Y), int32(Z),
	               [&](const FVoxelDataOctreeBase& Octree)
	               {
		               if (Octree.IsLeaf() && Octree.AsLeaf().GetData<FVoxelValue>().HasData())
		               {
			               if (bIsGeneratorValue) *bIsGeneratorValue = false;
			               return FVoxelDataUtilities::MakeBilinearInterpolatedData(*this).GetValue(X, Y, Z, LOD);
		               }
		               else
		               {
			               if (bIsGeneratorValue) *bIsGeneratorValue = true;
			               return Octree.GetFromGeneratorAndAssets<v_flt, v_flt>(*Data.Generator, X, Y, Z, LOD);
		               }
	               });
}

template<typename TData>
template<typename T>
FORCEINLINE T TVoxelDataAccelerator<TData>::Get(int32 X, int32 Y, int32 Z, int32 LOD) const
{
	// Clamp to world, to avoid un-editable border
	Data.ClampToWorld(X, Y, Z);

	return GetImpl(X, Y, Z,
	               [&](const FVoxelDataOctreeBase& Octree)
	               {
		               return Octree.Get<T>(*Data.Generator, X, Y, Z, LOD);
	               });
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<typename TData>
template<typename T>
auto TVoxelDataAccelerator<TData>::GetImpl(int32 X, int32 Y, int32 Z, T UseOctree) const
{
	ACCELERATOR_STAT(NumGet++);

	// Each caller should clamp the coordinates
	ensureVoxelSlow(Data.IsInWorld(X, Y, Z));

	ensureMsgfVoxelSlowNoSideEffects(!bUseAcceleratorMap || Bounds.Contains(X, Y, Z), TEXT("(%d, %d, %d) is not in %s!"), X, Y, Z, *Bounds.ToString());

	FVoxelDataOctreeBase* Octree = GetOctreeFromCache_CheckTopOnly(X, Y, Z);

	if (Octree)
	{
		// Fast path: top cache is a hit
		return UseOctree(*Octree);
	}

	Octree = GetOctreeFromCache_CheckAll(X, Y, Z);
	checkVoxelSlow(!Octree || Octree->IsInOctree(X, Y, Z));

	if (Octree)
	{
		// Cache hit
		return UseOctree(*Octree);
	}

	if (bUseAcceleratorMap)
	{
		Octree = GetOctreeFromMap(X, Y, Z);
	}
	
	if (!Octree)
	{
		// Need to get the octree for ItemHolders, even if there is no leaf
		Octree = &FVoxelOctreeUtilities::GetBottomNode(Data.GetOctree(), X, Y, Z);
	}

	checkVoxelSlow(Octree);
	StoreOctreeInCache(*Octree);

	return UseOctree(*Octree);
}

template<typename TData>
template<typename T, typename TLambda>
bool TVoxelDataAccelerator<TData>::SetImpl(int32 X, int32 Y, int32 Z, TLambda EditValue) const
{
	static_assert(!bIsConst, "Calling Set on a const data accelerator!");

	ACCELERATOR_STAT(NumSet++);

	ensureVoxelSlowNoSideEffects(!bUseAcceleratorMap || Bounds.Contains(X, Y, Z));

	FVoxelDataOctreeBase* Octree;

	const auto DoSet = [&]()
	{
		checkVoxelSlow(Octree);
		checkVoxelSlow(Octree->IsLeaf());
		checkVoxelSlow(Octree->IsInOctree(X, Y, Z));
		auto Iterate = [&](auto Lambda) { Lambda(X, Y, Z); };
		auto Apply = [&](int32, int32, int32, T& Value) { EditValue(Value); };
		FVoxelDataOctreeSetter::Set<T>(Data, Octree->AsLeaf(), Iterate, Apply);
	};

	Octree = GetOctreeFromCache_CheckTopOnly(X, Y, Z);

	// Set must be applied on leaves
	if (Octree && Octree->IsLeaf())
	{
		// Fast path: top cache is a hit
		DoSet();
		return true;
	}

	Octree = GetOctreeFromCache_CheckAll(X, Y, Z);

	// Set must be applied on leaves
	if (!Octree || !Octree->IsLeaf())
	{
		// No need to check IsInWorld if we get a hit, so check now instead
		if (!Data.IsInWorld(X, Y, Z))
		{
			ACCELERATOR_STAT(NumOutOfWorld++);
			return false;
		}
		
		if (bUseAcceleratorMap)
		{
			Octree = GetOctreeFromMap(X, Y, Z);
		}

		if (!Octree)
		{
			auto& Node = FVoxelOctreeUtilities::GetBottomNode(Data.GetOctree(), X, Y, Z);
			// Not true if it was edited outside of this accelerator // ensureVoxelSlowNoSideEffects(!bUseAcceleratorMap || !Node.IsLeaf());
			Octree = FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(Node, X, Y, Z);

			if (bUseAcceleratorMap)
			{
				const FIntVector HashPosition = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), DATA_CHUNK_SIZE);
				ensureVoxelSlowNoSideEffects(AcceleratorMap.IsUnique());
				AcceleratorMap->Add(HashPosition, &Octree->AsLeaf());
			}
		}

		StoreOctreeInCache(*Octree);
	}
	checkVoxelSlow(Octree);
	checkVoxelSlow(Octree->IsLeaf());

	DoSet();
	return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<typename TData>
FORCEINLINE FVoxelDataOctreeBase* TVoxelDataAccelerator<TData>::GetOctreeFromCache_CheckTopOnly(int32 X, int32 Y, int32 Z) const
{
	ACCELERATOR_STAT(NumCacheTopAccess++);
	if (CacheEntries.Num() == 0)
	{
		ACCELERATOR_STAT(NumCacheTopMiss++);
		return nullptr;
	}

	auto* Octree = CacheEntries.GetData()[0].Octree;
	// If we are const, octrees are not allowed to change and have children
	ensureVoxelSlowNoSideEffects(!bIsConst || Octree->IsLeafOrHasNoChildren());
	if (Octree->IsInOctree(X, Y, Z) && (bIsConst || Octree->IsLeafOrHasNoChildren()))
	{
		return Octree;
	}
	else
	{
		ACCELERATOR_STAT(NumCacheTopMiss++);
		return nullptr;
	}
}

template<typename TData>
FVoxelDataOctreeBase* TVoxelDataAccelerator<TData>::GetOctreeFromCache_CheckAll(int32 X, int32 Y, int32 Z) const
{
	checkVoxelSlow(!GetOctreeFromCache_CheckTopOnly(X, Y, Z) || !GetOctreeFromCache_CheckTopOnly(X, Y, Z)->IsLeaf());
	ACCELERATOR_STAT(NumCacheAllAccess++);
	for (int32 Index = 1; Index < CacheEntries.Num(); Index++)
	{
		ensureVoxelSlowNoSideEffects(CacheEntries[Index - 1].LastAccessTime > CacheEntries[Index].LastAccessTime);
	}
	for (int32 Index = 1; Index < CacheEntries.Num(); Index++)
	{
		auto& CacheEntry = CacheEntries.GetData()[Index];
		checkVoxelSlow(CacheEntry.Octree);
		ensureVoxelSlowNoSideEffects(!bIsConst || CacheEntry.Octree->IsLeafOrHasNoChildren());
		if (CacheEntry.Octree->IsInOctree(X, Y, Z) && (bIsConst || CacheEntry.Octree->IsLeafOrHasNoChildren()))
		{
			CacheEntry.LastAccessTime = ++GlobalTime;
			auto* Octree = CacheEntry.Octree; // Need to cache the ptr, as CacheEntry is going to be modified during the sorting

			// Sort the cache entries
			for (int32 SortIndex = Index; SortIndex > 0; SortIndex--)
			{
				CacheEntries.SwapMemory(SortIndex, SortIndex - 1);
			}

			return Octree;
		}
	}
	ACCELERATOR_STAT(NumCacheAllMiss++);
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<typename TData>
FVoxelDataOctreeBase* TVoxelDataAccelerator<TData>::GetOctreeFromMap(int32 X, int32 Y, int32 Z) const
{
	checkVoxelSlow(bUseAcceleratorMap);

	ACCELERATOR_STAT(NumMapAccess++);
	const FIntVector HashPosition = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), DATA_CHUNK_SIZE);
	auto* Result = AcceleratorMap->FindRef(HashPosition);
	ACCELERATOR_STAT(if (!Result) NumMapMiss++);
	return Result;
}

template<typename TData>
void TVoxelDataAccelerator<TData>::StoreOctreeInCache(FVoxelDataOctreeBase& Octree) const
{
	if (CacheSize <= 0) return;
	FCacheEntry CacheEntry;
	CacheEntry.Octree = &Octree;
	CacheEntry.LastAccessTime = ++GlobalTime;
	if (CacheEntries.Num() == CacheSize)
	{
		// Limit cache size
		CacheEntries.Pop(false);
	}
	CacheEntries.Insert(CacheEntry, 0);
}

template<typename TData>
TVoxelSharedRef<TMap<FIntVector, FVoxelDataOctreeLeaf*>> TVoxelDataAccelerator<TData>::GetAcceleratorMap(const FVoxelData& Data, const FVoxelIntBox& Bounds)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const auto AcceleratorMap = MakeVoxelShared<FAcceleratorMap>();
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](auto& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForRead());
		AcceleratorMap->Add(Leaf.GetMin() / DATA_CHUNK_SIZE, &Leaf);
	});
	AcceleratorMap->Compact();
	return AcceleratorMap;
}

#undef ACCELERATOR_STAT