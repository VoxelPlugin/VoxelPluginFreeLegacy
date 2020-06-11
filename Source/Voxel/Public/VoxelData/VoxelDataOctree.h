// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelOctree.h"
#include "VoxelQueryZone.h"
#include "VoxelSharedMutex.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"
#include "VoxelData/VoxelDataOctreeLeafData.h"
#include "VoxelData/VoxelDataOctreeLeafUndoRedo.h"
#include "VoxelData/VoxelDataOctreeLeafMultiplayer.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Data Octrees Memory"), STAT_VoxelDataOctreesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Voxel Data Octrees Count"), STAT_VoxelDataOctreesCount, STATGROUP_VoxelCounters, VOXEL_API);

class FVoxelWorldGeneratorInstance;

class VOXEL_API IVoxelData : public IVoxelDataOctreeMemory
{
public:
	const int32 Depth;
	const FVoxelIntBox WorldBounds;
	const bool bEnableMultiplayer;
	const bool bEnableUndoRedo;
	const TVoxelSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator;

	IVoxelData(
		int32 Depth,
		const FVoxelIntBox& WorldBounds,
		bool bEnableMultiplayer,
		bool bEnableUndoRedo,
		const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& VoxelWorldGeneratorInstance);
};

namespace FVoxelDataOctreeUtilities
{
	FORCEINLINE FVoxelCellIndex IndexFromCoordinates(int32 X, int32 Y, int32 Z)
	{
		checkVoxelSlow(0 <= X && X < DATA_CHUNK_SIZE && 0 <= Y && Y < DATA_CHUNK_SIZE && 0 <= Z && Z < DATA_CHUNK_SIZE);
		return X + DATA_CHUNK_SIZE * Y + DATA_CHUNK_SIZE * DATA_CHUNK_SIZE * Z;
	}
	FORCEINLINE FIntVector CoordinatesFromIndex(FVoxelCellIndex Index)
	{
		return
		{
			Index % DATA_CHUNK_SIZE,
			(Index / DATA_CHUNK_SIZE) % DATA_CHUNK_SIZE,
			(Index / (DATA_CHUNK_SIZE * DATA_CHUNK_SIZE))
		};
	}
	FORCEINLINE FVoxelCellIndex IndexFromGlobalCoordinates(const FIntVector& Min, int32 X, int32 Y, int32 Z)
	{
		X -= Min.X;
		Y -= Min.Y;
		Z -= Min.Z;
		return IndexFromCoordinates(X, Y, Z);
	}
}

class VOXEL_API FVoxelDataOctreeBase : public TVoxelOctreeBase<DATA_CHUNK_SIZE>
{
public:
	FVoxelDataOctreeBase(uint8 Height, const FIntVector& Position)
		: TVoxelOctreeBase(Height, Position)
	{
		INC_DWORD_STAT_BY(STAT_VoxelDataOctreesCount, 1);
	}
	~FVoxelDataOctreeBase()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelDataOctreesCount, 1);
	}

public:
	class FVoxelDataOctreeParent& AsParent();
	const FVoxelDataOctreeParent& AsParent() const;

	class FVoxelDataOctreeLeaf& AsLeaf();
	const FVoxelDataOctreeLeaf& AsLeaf() const;

	bool IsLeafOrHasNoChildren() const;

public:
	template<typename T>
	T Get(const FVoxelWorldGeneratorInstance& WorldGenerator, int32 X, int32 Y, int32 Z, int32 LOD) const;
	template<typename T>
	T GetCustomOutput(const FVoxelWorldGeneratorInstance& WorldGenerator, T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
	template<typename T, typename U = int32>
	T GetFromGeneratorAndAssets(const FVoxelWorldGeneratorInstance& WorldGenerator, U X, U Y, U Z, int32 LOD) const;
	template<typename T>
	void GetFromGeneratorAndAssets(const FVoxelWorldGeneratorInstance& WorldGenerator, TVoxelQueryZone<T>& QueryZone, int32 LOD) const;

public:
#if DO_THREADSAFE_CHECKS
	bool IsLockedForRead() const { return Mutex.IsLockedForRead() || (Parent && Parent->IsLockedForRead()); }
	bool IsLockedForWrite() const { return Mutex.IsLockedForWrite() || (Parent && Parent->IsLockedForWrite()); }
#endif

public:
	FVoxelPlaceableItemHolder& GetItemHolder() { return *ItemHolder; }
	const FVoxelPlaceableItemHolder& GetItemHolder() const { return *ItemHolder; }

private:
	// Always valid on a node with no children
	TUniquePtr<FVoxelPlaceableItemHolder> ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>();
	FVoxelSharedMutex Mutex;
#if DO_THREADSAFE_CHECKS
	FVoxelDataOctreeBase* Parent = nullptr;
#endif
	
	friend class FVoxelDataOctreeLocker;
	friend class FVoxelDataOctreeUnlocker;
	friend class FVoxelDataOctreeParent;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelDataOctreeLeaf : public TVoxelOctreeLeaf<FVoxelDataOctreeBase>
{
public:
	FVoxelDataOctreeLeaf(const FVoxelDataOctreeBase& Parent, uint8 ChildIndex)
		: TVoxelOctreeLeaf(Parent, ChildIndex)
	{
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctreeLeaf));
	}
	~FVoxelDataOctreeLeaf()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctreeLeaf));
	}

	TVoxelDataOctreeLeafData<FVoxelValue> Values;
	TVoxelDataOctreeLeafData<FVoxelMaterial> Materials;
	TVoxelDataOctreeLeafData<FVoxelFoliage> Foliage;

	TUniquePtr<FVoxelDataOctreeLeafUndoRedo> UndoRedo;
	TUniquePtr<FVoxelDataOctreeLeafMultiplayer> Multiplayer;

public:
	template<typename T>
	FORCEINLINE void InitForEdit(const IVoxelData& Data)
	{
		using TNotConst = typename TRemoveConst<T>::Type;
		
		TVoxelDataOctreeLeafData<TNotConst>& DataHolder = GetData<TNotConst>();
		if (!DataHolder.GetDataPtr())
		{
			if (DataHolder.IsSingleValue())
			{
				DataHolder.ExpandSingleValue(Data);
			}
			else
			{
				DataHolder.CreateDataPtr(Data);
				{
					TVoxelQueryZone<TNotConst> QueryZone(GetBounds(), DataHolder.GetDataPtr());
					GetFromGeneratorAndAssets(*Data.WorldGenerator, QueryZone, 0);
				}
			}
		}
		if (!TIsConst<T>::Value)
		{
			if (Data.bEnableMultiplayer && !Multiplayer.IsValid())
			{
				Multiplayer = MakeUnique<FVoxelDataOctreeLeafMultiplayer>();
			}
			if (Data.bEnableUndoRedo && !UndoRedo.IsValid())
			{
				UndoRedo = MakeUnique<FVoxelDataOctreeLeafUndoRedo>(*this);
			}
		}
	}

public:
	template<typename T> FORCEINLINE       TVoxelDataOctreeLeafData<T>& GetData()       { return FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*this); }
	template<typename T> FORCEINLINE const TVoxelDataOctreeLeafData<T>& GetData() const { return FVoxelUtilities::TValuesMaterialsSelector<T>::Get(*this); }
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelDataOctreeParent : public TVoxelOctreeParent<FVoxelDataOctreeBase, FVoxelDataOctreeLeaf, FVoxelDataOctreeParent>
{
public:
	explicit FVoxelDataOctreeParent(uint8 Height)
		: TVoxelOctreeParent(Height)
	{
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctreeParent));
	}
	~FVoxelDataOctreeParent()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctreeParent));
	}
	FVoxelDataOctreeParent(const FVoxelDataOctreeParent& Parent, uint8 ChildIndex)
		: TVoxelOctreeParent(Parent, ChildIndex)
	{
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctreeParent));
	}

	void CreateChildren();
	void DestroyChildren();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelDataOctreeSetter
{
	template<typename T, typename T1, typename T2>
	static void Set(
		IVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		T1 Iterate, T2 Apply)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		ensureThreadSafe(Leaf.IsLockedForWrite());

		const auto& DisableEditsBoxes = Leaf.GetItemHolder().GetItems(EVoxelPlaceableItemId::DisableEditsBox);
			
		const auto DoWork = [&](auto NeedToCheckCanEdit, auto EnableMultiplayer, auto EnableUndoRedo)
		{
			Leaf.InitForEdit<T>(Data);

			const FIntVector Min = Leaf.GetMin();
			T* RESTRICT DataPtr = Leaf.GetData<T>().GetDataPtr();
			
			Iterate([&](int32 X, int32 Y, int32 Z)
			{
				checkVoxelSlow(Leaf.IsInOctree(X, Y, Z));
				if (NeedToCheckCanEdit)
				{
					for (auto* Item : DisableEditsBoxes)
					{
						if (Item->Bounds.Contains(X, Y, Z))
						{
							return;
						}
					}
				}

				const uint32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);
				T& Ref = DataPtr[Index];
				T OldValue = Ref;

				Apply(X, Y, Z, Ref);

				if (OldValue != Ref)
				{
					Leaf.GetData<T>().SetDirty(Data);
					if (EnableMultiplayer) Leaf.Multiplayer->MarkIndexDirty<T>(Index);
					if (EnableUndoRedo) Leaf.UndoRedo->SavePreviousValue(Index, OldValue);
				}
			});
		};

		FVoxelUtilities::StaticBranch(DisableEditsBoxes.Num() > 0, Data.bEnableMultiplayer, Data.bEnableUndoRedo, DoWork);
	}
	template<typename TA, typename TB, typename T1, typename T2>
	static void Set(
		IVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf, 
		T1 Iterate, T2 Apply)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();
		
		ensureThreadSafe(Leaf.IsLockedForWrite());

		const auto& DisableEditsBoxes = Leaf.GetItemHolder().GetItems(EVoxelPlaceableItemId::DisableEditsBox);
		
		const auto DoWork = [&](auto NeedToCheckCanEdit, auto EnableMultiplayer, auto EnableUndoRedo)
		{
			Leaf.InitForEdit<TA>(Data);
			Leaf.InitForEdit<TB>(Data);

			const FIntVector Min = Leaf.GetMin();
			TA* RESTRICT DataPtrA = Leaf.GetData<TA>().GetDataPtr();
			TB* RESTRICT DataPtrB = Leaf.GetData<TB>().GetDataPtr();
			
			Iterate([&](int32 X, int32 Y, int32 Z)
			{
				checkVoxelSlow(Leaf.IsInOctree(X, Y, Z));
				if (NeedToCheckCanEdit)
				{
					for (auto* Item : DisableEditsBoxes)
					{
						if (Item->Bounds.Contains(X, Y, Z))
						{
							return;
						}
					}
				}
				
				const uint32 Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(Min, X, Y, Z);

				TA& RefA = DataPtrA[Index];
				TB& RefB = DataPtrB[Index];
				TA OldValueA = RefA;
				TB OldValueB = RefB;

				Apply(X, Y, Z, RefA, RefB);

				if (OldValueA != RefA)
				{
					Leaf.GetData<TA>().SetDirty(Data);
					if (EnableMultiplayer) Leaf.Multiplayer->MarkIndexDirty<TA>(Index);
					if (EnableUndoRedo) Leaf.UndoRedo->SavePreviousValue(Index, OldValueA);
				}
				if (OldValueB != RefB)
				{
					Leaf.GetData<TB>().SetDirty(Data);
					if (EnableMultiplayer) Leaf.Multiplayer->MarkIndexDirty<TB>(Index);
					if (EnableUndoRedo) Leaf.UndoRedo->SavePreviousValue(Index, OldValueB);
				}
			});
		};

		FVoxelUtilities::StaticBranch(DisableEditsBoxes.Num() > 0, Data.bEnableMultiplayer, Data.bEnableUndoRedo, DoWork);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FORCEINLINE FVoxelDataOctreeParent& FVoxelDataOctreeBase::AsParent()
{
	checkVoxelSlow(!IsLeaf());
	return static_cast<FVoxelDataOctreeParent&>(*this);
}

FORCEINLINE const FVoxelDataOctreeParent& FVoxelDataOctreeBase::AsParent() const
{
	checkVoxelSlow(!IsLeaf());
	return static_cast<const FVoxelDataOctreeParent&>(*this);
}

FORCEINLINE FVoxelDataOctreeLeaf& FVoxelDataOctreeBase::AsLeaf()
{
	checkVoxelSlow(IsLeaf());
	return static_cast<FVoxelDataOctreeLeaf&>(*this);
}

FORCEINLINE const FVoxelDataOctreeLeaf& FVoxelDataOctreeBase::AsLeaf() const
{
	checkVoxelSlow(IsLeaf());
	return static_cast<const FVoxelDataOctreeLeaf&>(*this);
}

FORCEINLINE bool FVoxelDataOctreeBase::IsLeafOrHasNoChildren() const
{
	return IsLeaf() || !AsParent().HasChildren();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
T FVoxelDataOctreeBase::Get(const FVoxelWorldGeneratorInstance& WorldGenerator, int32 X, int32 Y, int32 Z, int32 LOD) const
{
	checkVoxelSlow(IsLeafOrHasNoChildren());
	ensureThreadSafe(IsLockedForRead());
	if (IsLeaf())
	{
		auto& Data = AsLeaf().GetData<T>();
		if (Data.GetDataPtr())
		{
			return Data.GetDataPtr()[FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(GetMin(), X, Y, Z)];
		}
		if (Data.IsSingleValue())
		{
			return Data.GetSingleValue();
		}
	}
	return GetFromGeneratorAndAssets<T>(WorldGenerator, X, Y, Z, LOD);
}

// Foliage is an override of the generator values: as such it doesn't make sense to query it from the generator
template<>
FORCEINLINE FVoxelFoliage FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelFoliage>(const FVoxelWorldGeneratorInstance&, int32, int32, int32, int32) const
{
	return FVoxelFoliage::NotSet();
}
template<>
inline void FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelFoliage>(const FVoxelWorldGeneratorInstance&, TVoxelQueryZone<FVoxelFoliage>& QueryZone, int32) const
{
	for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
			{
				QueryZone.Set(X, Y, Z, FVoxelFoliage::NotSet());
			}
		}
	}
}