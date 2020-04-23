// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataUtilities.h"

namespace FVoxelDataAcceleratorParameters
{
	VOXEL_API int32 GetDefaultCacheSize();
	VOXEL_API bool GetUseAcceleratorMap();
	VOXEL_API bool GetShowStats();
}

#if VOXEL_DATA_ACCELERATOR_STATS
#define ACCELERATOR_STAT(X) X
#else
#define ACCELERATOR_STAT(X)
#endif
	
template<typename TData>
class TVoxelDataAccelerator
{
public:
	TData& Data;
	const FIntBox Bounds;
	const int32 CacheSize;

	static constexpr bool bIsConst = TIsConst<TData>::Value;

	TVoxelDataAccelerator(TData& Data, const FIntBox& Bounds, int32 CacheSize = FVoxelDataAcceleratorParameters::GetDefaultCacheSize())
		: Data(Data)
		, Bounds(Bounds)
		, CacheSize(CacheSize)
		, AcceleratorMap(GetAcceleratorMap(Data, Bounds))
	{
		CacheEntries.Reserve(CacheSize);
	}
	~TVoxelDataAccelerator()
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

public:
	template<typename T>
	FORCEINLINE T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
	{
		return GetImpl(X, Y, Z,
			[&](const FVoxelDataOctreeBase& Octree)
			{
				return Octree.GetCustomOutput<T>(*Data.WorldGenerator, DefaultValue, Name, X, Y, Z, LOD);
			},
			[&](const FVoxelWorldGeneratorInstance& WorldGenerator)
			{
				return WorldGenerator.GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack::Empty);
			});
	}

public:
	FORCEINLINE v_flt GetFloatValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, bool* bIsGeneratorValue = nullptr) const
	{
		return GetImpl(int32(X), int32(Y), int32(Z),
			[&](const FVoxelDataOctreeBase& Octree)
			{
				if (Octree.IsLeaf() && (
					Octree.AsLeaf().GetData<FVoxelValue>().GetDataPtr() ||
					Octree.AsLeaf().GetData<FVoxelValue>().IsSingleValue()))
				{
					if (bIsGeneratorValue) *bIsGeneratorValue = false;
					return FVoxelDataUtilities::MakeBilinearInterpolatedData(*this).GetValue(X, Y, Z, LOD);
				}
				else
				{
					if (bIsGeneratorValue) *bIsGeneratorValue = true;
					return Octree.GetFromGeneratorAndAssets<v_flt, v_flt>(*Data.WorldGenerator, X, Y, Z, LOD);
				}
			},
			[&](const FVoxelWorldGeneratorInstance& WorldGenerator)
			{
				if (bIsGeneratorValue) *bIsGeneratorValue = true;
				return WorldGenerator.GetValue(X, Y, Z, LOD, FVoxelItemStack::Empty);
			});
	}
	FORCEINLINE v_flt GetFloatValue(const FVector& P, int32 LOD, bool* bIsGeneratorValue = nullptr) const
	{
		return GetFloatValue(P.X, P.Y, P.Z, LOD, bIsGeneratorValue);
	}

public:
	template<typename T>
	FORCEINLINE T Get(int32 X, int32 Y, int32 Z, int32 LOD) const
	{
		return GetImpl(X, Y, Z,
			[&](const FVoxelDataOctreeBase& Octree)
			{
				return Octree.Get<T>(*Data.WorldGenerator, X, Y, Z, LOD);
			},
			[&](const FVoxelWorldGeneratorInstance& WorldGenerator)
			{
				return WorldGenerator.Get<T>(X, Y, Z, LOD, FVoxelItemStack::Empty);
			});
	}
	template<typename T>
	FORCEINLINE T Get(const FIntVector& P, int32 LOD) const
	{
		return Get<T>(P.X, P.Y, P.Z, LOD);
	}

	FORCEINLINE FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelValue>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelValue GetValue(const FIntVector& P, int32 LOD) const { return Get<FVoxelValue>(P, LOD); }

	FORCEINLINE FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelMaterial>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelMaterial GetMaterial(const FIntVector& P, int32 LOD) const { return Get<FVoxelMaterial>(P, LOD); }

public:
	template<typename T>
	FORCEINLINE void Set(int32 X, int32 Y, int32 Z, const T& Value)
	{
		SetImpl<T>(X, Y, Z, [&](auto& InValue) { InValue = Value; });
	}
	template<typename T>
	FORCEINLINE void Set(const FIntVector& P, const T& Value)
	{
		Set<T>(P.X, P.Y, P.Z, Value);
	}

	template<typename TDummy = void>
	FORCEINLINE void SetValue(int32 X, int32 Y, int32 Z, FVoxelValue Value) { Set<FVoxelValue>(X, Y, Z, Value); }
	template<typename TDummy = void>
	FORCEINLINE void SetValue(const FIntVector& P, FVoxelValue Value) { Set<FVoxelValue>(P, Value); }

	template<typename TDummy = void>
	FORCEINLINE void SetMaterial(int32 X, int32 Y, int32 Z, FVoxelMaterial Material) { Set<FVoxelMaterial>(X, Y, Z, Material); }
	template<typename TDummy = void>
	FORCEINLINE void SetMaterial(const FIntVector& P, FVoxelMaterial Material) { Set<FVoxelMaterial>(P, Material); }

public:
	template<typename T, typename TLambda>
	FORCEINLINE void Edit(int32 X, int32 Y, int32 Z, TLambda Lambda)
	{
		SetImpl<T>(X, Y, Z, Lambda);
	}
	template<typename T, typename TLambda>
	FORCEINLINE void Edit(const FIntVector& P, TLambda Lambda)
	{
		Edit<T>(P.X, P.Y, P.Z, Lambda);
	}

	template<typename TLambda>
	FORCEINLINE void EditValue(int32 X, int32 Y, int32 Z, TLambda Lambda) { Edit<FVoxelValue>(X, Y, Z, Lambda); }
	template<typename TLambda>
	FORCEINLINE void EditValue(const FIntVector& P, TLambda Lambda) { Edit<FVoxelValue>(P, Lambda); }

	template<typename TLambda>
	FORCEINLINE void EditMaterial(int32 X, int32 Y, int32 Z, TLambda Lambda) { Edit<FVoxelMaterial>(X, Y, Z, Lambda); }
	template<typename TLambda>
	FORCEINLINE void EditMaterial(const FIntVector& P, TLambda Lambda) { Edit<FVoxelMaterial>(P, Lambda); }

private:
	struct FCacheEntry
	{
		FVoxelDataOctreeBase* Octree;
		uint64 LastAccessTime;
	};
	mutable TArray<FCacheEntry> CacheEntries;
	mutable uint64 GlobalTime = 0;

#if VOXEL_DATA_ACCELERATOR_STATS
	mutable uint32 NumGet = 0;
	mutable uint32 NumSet = 0;

	mutable uint32 NumCacheTopAccess = 0;
	mutable uint32 NumCacheTopMiss = 0;

	mutable uint32 NumCacheAllAccess = 0;
	mutable uint32 NumCacheAllMiss = 0;

	mutable uint32 NumMapAccess = 0;
	mutable uint32 NumMapMiss = 0;
	
	mutable uint32 NumOutOfWorld = 0;
#endif

	// Map from Leaf.GetMin() to &Leaf
	mutable TMap<FIntVector, FVoxelDataOctreeLeaf*> AcceleratorMap;

	const bool bUseAcceleratorMap = FVoxelDataAcceleratorParameters::GetUseAcceleratorMap();

	template<typename T1, typename T2>
	auto GetImpl(int32 X, int32 Y, int32 Z, T1 UseOctree, T2 UseWorldGenerator) const
	{
		ACCELERATOR_STAT(NumGet++);

		ensureMsgfVoxelSlowNoSideEffects(Bounds.Contains(X, Y, Z), TEXT("(%d, %d, %d) is not in %s!"), X, Y, Z, *Bounds.ToString());
		
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
		
		// No need to check IsInWorld if we get a hit, so check now instead
		if (!Data.IsInWorld(X, Y, Z))
		{
			// Outside the world
			ACCELERATOR_STAT(NumOutOfWorld++);
			return UseWorldGenerator(*Data.WorldGenerator);
		}

		Octree = GetOctreeFromMap(X, Y, Z);
		if (!Octree)
		{
			// Need to get the octree for ItemHolders, even if there is no leaf
			Octree = &FVoxelOctreeUtilities::GetBottomNode(Data.GetOctree(), X, Y, Z);
		}

		checkVoxelSlow(Octree);
		StoreOctreeInCache(*Octree);

		return UseOctree(*Octree);
	}
	template<typename T, typename TLambda>
	void SetImpl(int32 X, int32 Y, int32 Z, TLambda EditValue) const
	{
		static_assert(!bIsConst, "Calling Set on a const data accelerator!");
		
		ACCELERATOR_STAT(NumSet++);

		ensureVoxelSlowNoSideEffects(Bounds.Contains(X, Y, Z));
		
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
			return;
		}
		
		Octree = GetOctreeFromCache_CheckAll(X, Y, Z);
		
		// Set must be applied on leaves
		if (!Octree || !Octree->IsLeaf())
		{
			// No need to check IsInWorld if we get a hit, so check now instead
			if (!Data.IsInWorld(X, Y, Z))
			{
				ACCELERATOR_STAT(NumOutOfWorld++);
				return;
			}
			
			Octree = GetOctreeFromMap(X, Y, Z);
			if (!Octree)
			{
				auto& Node = FVoxelOctreeUtilities::GetBottomNode(Data.GetOctree(), X, Y, Z);
				// Not true if it was edited outside of this accelerator // ensureVoxelSlowNoSideEffects(!bUseAcceleratorMap || !Node.IsLeaf());
				Octree = FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(Node, X, Y, Z);
				const FIntVector HashPosition = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), DATA_CHUNK_SIZE);
				AcceleratorMap.Add(HashPosition, &Octree->AsLeaf());
			}

			StoreOctreeInCache(*Octree);
		}
		checkVoxelSlow(Octree);
		checkVoxelSlow(Octree->IsLeaf());

		DoSet();
	}

	FORCEINLINE FVoxelDataOctreeBase* GetOctreeFromCache_CheckTopOnly(int32 X, int32 Y, int32 Z) const
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
	FVoxelDataOctreeBase* GetOctreeFromCache_CheckAll(int32 X, int32 Y, int32 Z) const
	{
		checkVoxelSlow(!GetOctreeFromCache_CheckTopOnly(X, Y, Z));
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
	FVoxelDataOctreeBase* GetOctreeFromMap(int32 X, int32 Y, int32 Z) const
	{
		if (!bUseAcceleratorMap) return nullptr;

		ACCELERATOR_STAT(NumMapAccess++);
		const FIntVector HashPosition = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), DATA_CHUNK_SIZE);
		auto* Result = AcceleratorMap.FindRef(HashPosition);
		ACCELERATOR_STAT(if (!Result) NumMapMiss++);
		return Result;
	}	
	void StoreOctreeInCache(FVoxelDataOctreeBase& Octree) const
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

	static TMap<FIntVector, FVoxelDataOctreeLeaf*> GetAcceleratorMap(const FVoxelData& Data, const FIntBox& Bounds)
	{
		VOXEL_FUNCTION_COUNTER();
		
		TMap<FIntVector, FVoxelDataOctreeLeaf*> AcceleratorMap;
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](auto& Leaf)
		{
			ensureThreadSafe(Leaf.IsLockedForRead());
			AcceleratorMap.Add(Leaf.GetMin() / DATA_CHUNK_SIZE, &Leaf);
		});
		AcceleratorMap.Compact();
		return AcceleratorMap;
	}
};

#undef ACCELERATOR_STAT

class FVoxelMutableDataAccelerator : public TVoxelDataAccelerator<FVoxelData>
{
public:
	using TVoxelDataAccelerator<FVoxelData>::TVoxelDataAccelerator;
};

class FVoxelConstDataAccelerator : public TVoxelDataAccelerator<const FVoxelData>
{
public:
	using TVoxelDataAccelerator<const FVoxelData>::TVoxelDataAccelerator;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE FVoxelFoliage FVoxelWorldGeneratorInstance::Get<FVoxelFoliage>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return FVoxelFoliage::NotSet();
}