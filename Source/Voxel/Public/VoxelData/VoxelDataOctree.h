// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelOctree.h"
#include "VoxelDiff.h"
#include "VoxelGlobals.h"
#include "VoxelWorldGenerator.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelData/VoxelDataCell.h"
#include "VoxelData/VoxelSharedMutex.h"

DECLARE_MEMORY_STAT(TEXT("Voxel Data Octrees Memory"), STAT_VoxelDataOctreesMemory, STATGROUP_VoxelMemory);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Voxel Data Octrees Count"), STAT_VoxelDataOctreesCount, STATGROUP_VoxelMemory);

class FVoxelData;
class FVoxelPlaceableItem;
class FVoxelSaveLoader;
class FVoxelSaveBuilder;
class FVoxelDataOctree;

using FVoxelMap = TMap<FIntVector, const FVoxelDataOctree*>;
using FVoxelLockedOctrees = TArray<FVoxelId>;

/**
 * Octree that holds modified values & materials
 */
class VOXEL_API FVoxelDataOctree : public TVoxelOctree<FVoxelDataOctree, VOXEL_CELL_SIZE>
{
public:
	FVoxelDataOctree(FVoxelData* Data, const FVoxelWorldGeneratorInstance* WorldGenerator, uint8 LOD, bool bEnableMultiplayer, bool bEnableUndoRedo)
		: TVoxelOctree(LOD)
		, Data(Data)
		, WorldGenerator(WorldGenerator)
		, bEnableMultiplayer(bEnableMultiplayer)
		, bEnableUndoRedo(bEnableUndoRedo)
	{
		check(LOD > 0);
		INC_DWORD_STAT_BY(STAT_VoxelDataOctreesCount, 1);
		INC_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctree));
	}
	FVoxelDataOctree(FVoxelDataOctree* Parent, uint8 ChildIndex)
		: TVoxelOctree(Parent, ChildIndex)
		, Data(Parent->Data)
		, WorldGenerator(Parent->WorldGenerator)
		, bEnableMultiplayer(Parent->bEnableMultiplayer)
		, bEnableUndoRedo(Parent->bEnableUndoRedo)
	{
		INC_DWORD_STAT_BY(STAT_VoxelDataOctreesCount, 1);
		INC_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctree));
	}

	~FVoxelDataOctree()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelDataOctreesCount, 1);
		DEC_MEMORY_STAT_BY(STAT_VoxelDataOctreesMemory, sizeof(FVoxelDataOctree));
	}

	inline bool IsCreated() const { checkVoxelSlow(LOD == 0); return Cell.IsValid(); }
	inline bool IsCacheOnly() const { checkVoxelSlow(LOD == 0); return !IsCreated() || (!Cell->IsArrayDirty<FVoxelValue>() && !Cell->IsArrayDirty<FVoxelMaterial>()); }
	inline bool IsCached() const { checkVoxelSlow(LOD == 0); return IsCreated() && IsCacheOnly(); }
		
	inline bool ShouldBeCached(uint32 Threshold) const { return NumberOfWorldGeneratorReadsSinceLastCache > Threshold; }
	inline uint32 GetCachePriority() const { return LastAccessTime; }

	inline bool IsManuallyCached() const { return IsCached() && bIsManuallyCached; }
	
	inline bool AreValuesCreated() const { return IsCreated() && Cell->GetArray<FVoxelValue>(); }
	inline bool AreMaterialsCreated() const { return IsCreated() && Cell->GetArray<FVoxelMaterial>(); }
	
	bool AreBoundsCached(const FIntBox& Bounds);
	void CacheBounds(const FIntBox& Bounds, bool bIsManualCache, bool bCacheValues, bool bCacheMaterials);

	void Cache(bool bIsManualCache, bool bCacheValues, bool bCacheMaterials);
	void ClearCache();
	void ClearManualCache();	
	
	struct CacheElement
	{
		bool bIsCached;
		uint32 Priority;
		FVoxelDataOctree* Octree;
	};
	void GetOctreesToCacheAndExistingCachedOctrees(
		uint32 Time,
		uint32 Threshold, 
		TArray<CacheElement>& OutOctreesToCacheAndCachedOctrees,
		TArray<FVoxelDataOctree*>& OutOctreesToSubdivide);

public:
	void GetMap(const FIntBox& Bounds, FVoxelMap& OutMap) const;

public:
	EVoxelEmptyState IsEmpty(const FIntBox& Bounds, int LOD) const;

	void GetValuesAndMaterials(FVoxelValue Values[], FVoxelMaterial Materials[], const FVoxelWorldGeneratorQueryZone& QueryZone, int QueryLOD);
	void GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* Value, FVoxelMaterial* Material, int QueryLOD) const;
	
	template<typename TValue>
	void SetValueOrMaterial(int X, int Y, int Z, const TValue& Value);

	template<typename TValue, typename F>
	void SetValueOrMaterialLambda(const FIntBox& Bounds, F Lambda);
	template<typename TValue, typename F, bool bTEnableMultiplayer, bool bTEnableUndoRedo>
	void SetValueOrMaterialLambdaInternal(const FIntBox& Bounds, F Lambda);

	template<bool bOnlyIfDirty, typename F>
	void CallLambdaOnValuesInBounds(const FIntBox& Bounds, F Lambda) const;

	void ClearData();

public:
	void GetCreatedChunksOverlappingBox(const FIntBox& Box, TArray<FVoxelDataOctree*>& OutOctrees);
	
public:
	/**
	 * Add dirty chunks to SaveList
	 * @param	SaveQueue				Queue to save chunks into. Sorted by increasing Id
	 */
	void Save(FVoxelSaveBuilder& Builder);
	/**
	 * Load chunks from Save list
	 * @param	SaveQueue				Queue to load chunks from. Sorted by decreasing Id (top is lowest Id)
	 * @param	OutBoundsToUpdate		The modified bounds
	 */
	void Load(int& Index, const FVoxelSaveLoader& Loader, TArray<FIntBox>& OutBoundsToUpdate);
		
public:
	template<EVoxelLockType LockType>
	bool TryLock(const FIntBox& Bounds, double TimeToTimeout, FVoxelLockedOctrees& OutIds, FString& InOutLockerName);

	template<EVoxelLockType LockType>
	void Unlock(FVoxelLockedOctrees& Ids);

	inline void LockTransactions() { TransactionsSection.Lock(); }

public:
	template<typename T>
	void AddItem(T* Item);


public:
	/**
	 * Add the current frame to the undo stack. Clear the redo stack
	 */
	void SaveFrame(int HistoryPosition);
	/**
	 * Undo one frame and add it to the redo stack. Current frame must be empty
	 */
	void Undo(int HistoryPosition, TArray<FIntBox>& OutBoundsToUpdate);
	/**
	 * Redo one frame and add it to the undo stack. Current frame must be empty
	 */
	void Redo(int HistoryPosition, TArray<FIntBox>& OutBoundsToUpdate);
	/**
	 * Clear all the frames
	 */
	void ClearFrames();
	/**
	 * Check that the current frame is empty (safe to call Undo/Redo)
	 */
	bool IsCurrentFrameEmpty() const;

public:
	void CreateChildren();
	void DestroyChildren();
	bool Compact(uint32& NumDeleted);

private:
	TUniquePtr<FVoxelDataCell> Cell;
	TUniquePtr<FVoxelDataCellUndoRedo> UndoRedoCell;
	TUniquePtr<FVoxelPlaceableItemHolder> ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>(); // Always valid on a leaf

	FVoxelData* const Data;
	const FVoxelWorldGeneratorInstance* const WorldGenerator;
	const bool bEnableMultiplayer : 1;
	const bool bEnableUndoRedo : 1;

	bool bIsManuallyCached : 1;
	uint32 NumberOfWorldGeneratorReadsSinceLastCache = 0;
	uint32 LastAccessTime = 0;

private:
	FVoxelSharedMutex Mutex;
	FCriticalSection TransactionsSection;
#if ENABLE_LOCKER_NAME
	FString LockerName;
	FCriticalSection LockerNameSection;
#endif

#if DO_THREADSAFE_CHECKS
	FVoxelDataOctree* Parent = nullptr;
	
	inline bool IsLockedForWrite() const
	{
		if (Mutex.IsLockedForWrite())
		{
			return true;
		}
		else
		{
			return Parent && Parent->IsLockedForWrite();
		}
	}
	inline bool IsLockedForRead() const
	{
		if (Mutex.IsLockedForRead())
		{
			return true;
		}
		else
		{
			return Parent && Parent->IsLockedForRead();
		}
	}
#endif

private:
	void Create();
	void Destroy();

	void CreateArrayAndInitFromWorldGenerator(bool bInitValues, bool bInitMaterials);
	
	template<typename T> inline void CreateArrayAndInitFromWorldGenerator();

private:
	inline FVoxelCellIndex IndexFromGlobalCoordinates(int X, int Y, int Z) const
	{
		X -= OctreeBounds.Min.X;
		Y -= OctreeBounds.Min.Y;
		Z -= OctreeBounds.Min.Z;
		return FVoxelDataCellUtilities::IndexFromCoordinates(X, Y, Z);
	}

	template<typename T>
	inline void AddEdit(FVoxelCellIndex Index, T& Old)
	{
		if (bEnableUndoRedo)
		{
			UndoRedoCell->AddEdit(Index, Old);
		}
	}
	template<typename T, bool bInEnableMultiplayer, bool bInEnableUndoRedo>
	inline void AddEdit(FVoxelCellIndex Index, T& Old)
	{
		if (bInEnableUndoRedo)
		{
			UndoRedoCell->AddEdit(Index, Old);
		}
	}

	inline bool CanEdit(int X, int Y, int Z)
	{		
		return true;
	}

	inline TArray<FIntBox> GetEditableBoxes(const FIntBox& Bounds)
	{
		TArray<FIntBox> Result;
		Result.Add(Bounds);
		return Result;
	}
};

inline void FVoxelDataOctree::GetValueAndMaterial(int X, int Y, int Z, FVoxelValue* Value, FVoxelMaterial* Material, int QueryLOD) const
{
	ensureThreadSafe(IsLockedForRead());
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	if (LOD > 0 || !IsCreated())
	{
		WorldGenerator->GetValueAndMaterial(X, Y, Z, Value, Material, QueryLOD, *ItemHolder);
	}
	else
	{
		int Index = IndexFromGlobalCoordinates(X, Y, Z);

		if (Value)
		{
			if (auto* Array = Cell->GetArray<FVoxelValue>())
			{
				*Value = Array[Index];
			}
			else
			{
				WorldGenerator->GetValueAndMaterial(X, Y, Z, Value, nullptr, QueryLOD, *ItemHolder);
			}
		}
		if (Material)
		{
			if (auto* Array = Cell->GetArray<FVoxelMaterial>())
			{
				*Material = Array[Index];
			}
			else
			{
				WorldGenerator->GetValueAndMaterial(X, Y, Z, nullptr, Material, QueryLOD, *ItemHolder);
			}
		}
	}
}

template<typename TValue>
inline void FVoxelDataOctree::SetValueOrMaterial(int X, int Y, int Z, const TValue& Value)
{
	ensureThreadSafe(IsLockedForWrite());
	check(IsLeaf());
	check(IsInOctree(X, Y, Z));

	if (LOD == 0)
	{
		if (!CanEdit(X, Y, Z))
		{
			return;
		}
		if (!IsCreated())
		{
			Create();
		}
		if (!Cell->GetArray<TValue>())
		{
			CreateArrayAndInitFromWorldGenerator<TValue>();
		}

		Cell->SetArrayAsDirty<TValue>();
		TValue* Array = Cell->GetArray<TValue>();
		FVoxelCellIndex Index = IndexFromGlobalCoordinates(X, Y, Z);

		AddEdit(Index, Array[Index]);

		Array[Index] = Value;
	}
	else
	{
		CreateChildren();
		GetChild(X, Y, Z).SetValueOrMaterial<TValue>(X, Y, Z, Value);
	}
}

template<typename TValue, typename F>
inline void FVoxelDataOctree::SetValueOrMaterialLambda(const FIntBox& Bounds, F Lambda)
{
	if (bEnableMultiplayer)
	{
		if (bEnableUndoRedo)
		{
			SetValueOrMaterialLambdaInternal<TValue, F, true, true>(Bounds, Lambda);
		}
		else
		{
			SetValueOrMaterialLambdaInternal<TValue, F, true, false>(Bounds, Lambda);
		}
	}
	else
	{
		if (bEnableUndoRedo)
		{
			SetValueOrMaterialLambdaInternal<TValue, F, false, true>(Bounds, Lambda);
		}
		else
		{
			SetValueOrMaterialLambdaInternal<TValue, F, false, false>(Bounds, Lambda);
		}
	}
}

template<typename TValue, typename F, bool bTEnableMultiplayer, bool bTEnableUndoRedo>
inline void FVoxelDataOctree::SetValueOrMaterialLambdaInternal(const FIntBox& Bounds, F Lambda)
{
	if (Bounds.Intersect(GetBounds()))
	{
		if (LOD == 0)
		{
			ensureThreadSafe(IsLockedForWrite());

			const TArray<FIntBox> LocalBoundsArray = GetEditableBoxes(Bounds.Overlap(GetBounds()));
			if (LocalBoundsArray.Num() == 0)
			{
				return;
			}
			if (!IsCreated())
			{
				Create();
			}
			if (!Cell->GetArray<TValue>())
			{
				CreateArrayAndInitFromWorldGenerator<TValue>();
			}

			Cell->SetArrayAsDirty<TValue>();
			TValue* Array = Cell->GetArray<TValue>();

			for (auto& LocalBounds : LocalBoundsArray)
			{
				const FIntVector& Offset = OctreeBounds.Min;
				const FIntVector Min = LocalBounds.Min - Offset;
				const FIntVector Max = LocalBounds.Max - Offset;

				for (int Z = Min.Z; Z < Max.Z; Z++)
				{
					for (int Y = Min.Y; Y < Max.Y; Y++)
					{
						for (int X = Min.X; X < Max.X; X++)
						{
							FVoxelCellIndex Index = FVoxelDataCellUtilities::IndexFromCoordinates(X, Y, Z);

							if (bTEnableMultiplayer || bTEnableUndoRedo)
							{
								TValue& Ref = Array[Index];
								TValue Old = Ref;

								Lambda(X + Offset.X, Y + Offset.Y, Z + Offset.Z, Ref);

								if (Ref != Old)
								{
									AddEdit<TValue, bTEnableMultiplayer, bTEnableUndoRedo>(Index, Old);
								}
							}
							else
							{
								Lambda(X + Offset.X, Y + Offset.Y, Z + Offset.Z, Array[Index]);
							}
						}
					}
				}
			}
		}
		else
		{
			if (IsLeaf())
			{
				CreateChildren();
			}

			for (auto& Child : GetChildren())
			{
				Child.SetValueOrMaterialLambdaInternal<TValue, F, bTEnableMultiplayer, bTEnableUndoRedo>(Bounds, Lambda);
			}
		}
	}
}

template<bool bOnlyIfDirty, typename F>
inline void FVoxelDataOctree::CallLambdaOnValuesInBounds(const FIntBox& Bounds, F Lambda) const
{
	if (Bounds.Intersect(OctreeBounds))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForRead());

			const bool bCreatedLOD0 = LOD == 0 && IsCreated();
			if (bCreatedLOD0 || !bOnlyIfDirty)
			{
				const FIntBox LocalBounds = Bounds.Overlap(GetBounds());

				auto* Values = bCreatedLOD0 ? Cell->GetArray<FVoxelValue>() : nullptr;
				auto* Materials = bCreatedLOD0 ? Cell->GetArray<FVoxelMaterial>() : nullptr;

				TArray<FVoxelValue> ValuesArray;
				TArray<FVoxelMaterial> MaterialsArray;
				bool bUsingValuesArray = false;
				bool bUsingMaterialsArray = false;


				if (!bOnlyIfDirty && (!Values || !Materials))
				{
					const FIntVector Size = LocalBounds.Size();
					const int Num = Size.X * Size.Y * Size.Z;

					if (!Values)
					{
						ValuesArray.SetNumUninitialized(Num);
					}
					if (!Materials)
					{
						MaterialsArray.SetNumUninitialized(Num);
					}

					WorldGenerator->GetValuesAndMaterials(
						Values ? nullptr : ValuesArray.GetData(),
						Materials ? nullptr : MaterialsArray.GetData(),
						FVoxelWorldGeneratorQueryZone(LocalBounds, Size, 0),
						0,
						*ItemHolder);

					if (!Values)
					{
						Values = ValuesArray.GetData();
						bUsingValuesArray = true;
					}
					if (!Materials)
					{
						Materials = MaterialsArray.GetData();
						bUsingMaterialsArray = true;
					}
				}

				if (Values || Materials)
				{
					const FIntVector& Offset = OctreeBounds.Min;
					const FIntVector Min = LocalBounds.Min;
					const FIntVector Max = LocalBounds.Max;

					int ArrayIndex = 0;
					for (int Z = Min.Z; Z < Max.Z; Z++)
					{
						for (int Y = Min.Y; Y < Max.Y; Y++)
						{
							for (int X = Min.X; X < Max.X; X++)
							{
								int ValuesIndex = bUsingValuesArray ? ArrayIndex : FVoxelDataCellUtilities::IndexFromCoordinates(X, Y, Z);
								int MaterialsIndex = bUsingMaterialsArray ? ArrayIndex : FVoxelDataCellUtilities::IndexFromCoordinates(X, Y, Z);
								Lambda(
									X + Offset.X,
									Y + Offset.Y,
									Z + Offset.Z,
									(const FVoxelValue   *)(Values    ? &Values   [ValuesIndex   ] : nullptr),
									(const FVoxelMaterial*)(Materials ? &Materials[MaterialsIndex] : nullptr));
								ArrayIndex++;
							}
						}
					}
				}
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.CallLambdaOnValuesInBounds<bOnlyIfDirty>(Bounds, Lambda);
			}
		}
	}
}

template<typename T>
inline void FVoxelDataOctree::AddItem(T* Item)
{
	if (Item->Bounds.Intersect(GetBounds()))
	{
		if (IsLeaf())
		{
			ensureThreadSafe(IsLockedForWrite());

			ItemHolder->AddItem<T>(Item);

			if (ItemHolder->Num() > MAX_PLACEABLE_ITEMS_PER_OCTREE && LOD > 0)
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


template<> inline void FVoxelDataOctree::CreateArrayAndInitFromWorldGenerator<FVoxelValue>()
{
	CreateArrayAndInitFromWorldGenerator(true, false);
}

template<> inline void FVoxelDataOctree::CreateArrayAndInitFromWorldGenerator<FVoxelMaterial>()
{
	CreateArrayAndInitFromWorldGenerator(false, true);
}