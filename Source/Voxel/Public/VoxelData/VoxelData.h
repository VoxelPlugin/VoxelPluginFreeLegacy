// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelRange.h"
#include "IntBox.h"
#include "VoxelDiff.h"
#include "VoxelConfigEnums.h"
#include "VoxelQueryZone.h"
#include "VoxelOctreeUtilities.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelWorldGeneratorInstance.inl"
#include "VoxelData/VoxelSave.h"
#include "VoxelData/VoxelDataOctree.h"

#include "Async/ParallelFor.h"

struct FVoxelChunkMesh;
class FVoxelData;
class AVoxelWorld;
class FVoxelWorldGeneratorInstance;
class FVoxelPlaceableItem;

class FVoxelDataLockInfo
{
public:
	~FVoxelDataLockInfo()
	{
		checkf(LockedOctrees.Num() == 0, TEXT("Data not unlocked by %s!"), *Name.ToString());
	}
	
	FVoxelDataLockInfo(const FVoxelDataLockInfo&) = delete;
	FVoxelDataLockInfo& operator=(const FVoxelDataLockInfo&) = delete;

private:
	FVoxelDataLockInfo() = default;
	
	FName Name;
	EVoxelLockType LockType = EVoxelLockType::Read;
	TArray<FVoxelOctreeId> LockedOctrees; // In depth first order
	
	friend class FVoxelData;
};

struct FVoxelDataSettings
{
	const int32 Depth;
	const FIntBox WorldBounds;
	const TVoxelSharedRef<FVoxelWorldGeneratorInstance> WorldGenerator;
	const bool bEnableMultiplayer;
	const bool bEnableUndoRedo;

	FVoxelDataSettings(const AVoxelWorld* World, EVoxelPlayType PlayType);
	FVoxelDataSettings(
		int32 Depth,
		const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& WorldGenerator,
		bool bEnableMultiplayer,
		bool bEnableUndoRedo);
	FVoxelDataSettings(
		const FIntBox& WorldBounds,
		const TVoxelSharedRef<FVoxelWorldGeneratorInstance>& WorldGenerator,
		bool bEnableMultiplayer,
		bool bEnableUndoRedo);
};

/**
 * Class that handle voxel data
 */
class VOXEL_API FVoxelData : public IVoxelData, public TVoxelSharedFromThis<FVoxelData>
{
private:
	explicit FVoxelData(const FVoxelDataSettings& Settings);

public:
	static TVoxelSharedRef<FVoxelData> Create(const FVoxelDataSettings& Settings, int32 DataOctreeInitialSubdivisionDepth = 0);
	~FVoxelData();
	
private:
	TUniquePtr<FVoxelDataOctreeParent> Octree;
	// Is locked as read when a lock is done
	// Lock as write to clear the octree, making sure no octrees are locked
	mutable FVoxelSharedMutex MainLock;

public:
	FORCEINLINE int32 Size() const
	{
		return DATA_CHUNK_SIZE << Depth;
	}
	FORCEINLINE FVoxelDataOctreeBase& GetOctree() const
	{
		return *Octree;
	}
	// NOTE: what if we query between WorldBounds.Max - 1 and WorldBounds.Max?
	template<typename T>
	FORCEINLINE bool IsInWorld(T X, T Y, T Z) const
	{
		return WorldBounds.ContainsTemplate(X, Y, Z);
	}
	template<typename T>
	FORCEINLINE bool IsInWorld(const T& P) const
	{
		return WorldBounds.ContainsTemplate(P);
	}

public:
	/**
	 * Lock the bounds
	 * @param	LockType			Read or write lock
	 * @param	Bounds				Bounds to lock
	 * @param	Name				The name of the task locking these bounds, for debug
	 */
	TUniquePtr<FVoxelDataLockInfo> Lock(EVoxelLockType LockType, const FIntBox& Bounds, FName Name) const;

	/**
	 * Unlock previously locked bounds
	 */
	void Unlock(TUniquePtr<FVoxelDataLockInfo> LockInfo) const;
	 	
public:	
	// Must NOT be locked. Will delete the entire octree & recreate one
	// Destroys all items
	void ClearData();

	// Will clear all the edited data. Keeps items
	// Requires write lock
	void ClearOctreeData(TArray<FIntBox>& OutBoundsToUpdate);

	// Requires write lock
	template<typename T>
	void CacheBounds(const FIntBox& Bounds);
	
	// Requires write lock
	template<typename T>
	void ClearCacheInBounds(const FIntBox& Bounds);
	
	// Requires write lock
	template<typename T>
	void CheckIsSingle(const FIntBox& Bounds);

	// Get the data in zone. Requires read lock
	template<typename T>
	void Get(TVoxelQueryZone<T>& QueryZone, int32 LOD) const;
	
	template<typename T>
	TArray<T> Get(const FIntBox& Bounds) const
	{
		TArray<T> Result;
		Result.SetNumUninitialized(Bounds.Count());
		TVoxelQueryZone<T> QueryZone(Bounds, Result);
		Get(QueryZone, 0);
		return Result;
	}

	// Will always use 8 threads
	template<typename T>
	TArray<T> ParallelGet(const FIntBox& Bounds, bool bForceSingleThread = false) const
	{
		VOXEL_FUNCTION_COUNTER();
		
		TArray<T> Result;
		Result.SetNumUninitialized(Bounds.Count());
		TVoxelQueryZone<T> QueryZone(Bounds, Result);

		const FIntVector Half = (Bounds.Min + Bounds.Max) / 2;
		ParallelFor(8, [&](int32 Index)
		{
			const FIntBox LocalBounds(
				FIntVector(
					(Index & 0x1) ? Half.X : Bounds.Min.X,
					(Index & 0x2) ? Half.Y : Bounds.Min.Y,
					(Index & 0x4) ? Half.Z : Bounds.Min.Z),
				FIntVector(
					(Index & 0x1) ? Bounds.Max.X : Half.X,
					(Index & 0x2) ? Bounds.Max.Y : Half.Y,
					(Index & 0x4) ? Bounds.Max.Z : Half.Z));
			auto LocalQueryZone = QueryZone.ShrinkTo(LocalBounds);
			Get(LocalQueryZone, 0);
		}, bForceSingleThread);
		
		return Result;
	}

	TArray<FVoxelValue> GetValues(const FIntBox& Bounds) const
	{
		return Get<FVoxelValue>(Bounds);
	}
	TArray<FVoxelMaterial> GetMaterials(const FIntBox& Bounds) const
	{
		return Get<FVoxelMaterial>(Bounds);
	}

	// Requires read lock
	TVoxelRange<FVoxelValue> GetValueRange(const FIntBox& Bounds, int32 LOD) const;

	FORCEINLINE bool IsEmpty(const FIntBox& Bounds, int32 LOD) const
	{
		const auto Range = GetValueRange(Bounds, LOD);
		return Range.Min.IsEmpty() == Range.Max.IsEmpty();
	}

	template<typename T>
	FORCEINLINE T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
	{
		if (IsInWorld(X, Y, Z))
		{
			auto& Node = FVoxelOctreeUtilities::GetBottomNode(GetOctree(), int32(X), int32(Y), int32(Z));
			return Node.GetCustomOutput<T>(*WorldGenerator, DefaultValue, Name, X, Y, Z, LOD);
		}
		else
		{
			return WorldGenerator->GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack::Empty);
		}
	}
	template<typename T, typename U>
	FORCEINLINE T GetCustomOutput(T DefaultValue, FName Name, const U& P, int32 LOD) const
	{
		return GetCustomOutput<T>(DefaultValue, Name, P.X, P.Y, P.Z, LOD);
	}
	
	TVoxelRange<v_flt> GetCustomOutputRange(TVoxelRange<v_flt> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD) const;
	
public:
	template<typename ...TArgs, typename F>
	void Set(const FIntBox& Bounds, F Apply)
	{
		if (!ensure(Bounds.IsValid())) return;
		FVoxelOctreeUtilities::IterateTreeInBounds(GetOctree(), Bounds, [&](FVoxelDataOctreeBase& Tree)
		{
			if (Tree.IsLeaf())
			{
				auto& Leaf = Tree.AsLeaf();
				ensureThreadSafe(Leaf.IsLockedForWrite());
				FVoxelDataOctreeSetter::Set<TArgs...>(*this, Leaf, [&](auto Lambda)
				{
					Leaf.GetBounds().Overlap(Bounds).Iterate(Lambda);
				}, Apply);
			}
			else
			{
				auto& Parent = Tree.AsParent();
				if (!Parent.HasChildren())
				{
					ensureThreadSafe(Parent.IsLockedForWrite());
					Parent.CreateChildren();
				}
			}
		});
	}
	
public:
	/**
	 * Getters/Setters
	 */
	// Set value or material at position depending on template argument (FVoxelValue or FVoxelMaterial)
	template<typename T>
	FORCEINLINE void Set(int32 X, int32 Y, int32 Z, const T& Value)
	{
		 if (IsInWorld(X, Y, Z))
		 {
			 auto Iterate = [&](auto Lambda) { Lambda(X, Y, Z); };
			 auto Apply = [&](int32, int32, int32, T& InValue) { InValue = Value; };
			 auto& Leaf = *FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(GetOctree(), X, Y, Z);
			 FVoxelDataOctreeSetter::Set<T>(*this, Leaf, Iterate, Apply);
		 }
	}
	template<typename T>
	FORCEINLINE void Set(const FIntVector& P, const T& Value)
	{
		Set<T>(P.X, P.Y, P.Z, Value);
	}

	template<typename T>
	FORCEINLINE T Get(int32 X, int32 Y, int32 Z, int32 LOD) const
	{
		if (IsInWorld(X, Y, Z))
		{
			auto& Node = FVoxelOctreeUtilities::GetBottomNode(GetOctree(), int32(X), int32(Y), int32(Z));
			return Node.Get<T>(*WorldGenerator, X, Y, Z, LOD);
		}
		else
		{
			return WorldGenerator->Get<T>(X, Y, Z, LOD, FVoxelItemStack::Empty);
		}
	}
	template<typename T>
	FORCEINLINE T Get(const FIntVector& P, int32 LOD) const
	{
		return Get<T>(P.X, P.Y, P.Z, LOD);
	}

	// Get the value at position. Requires read lock
	FORCEINLINE FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelValue>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelValue GetValue(const FIntVector& P      , int32 LOD) const { return Get<FVoxelValue>(P,       LOD); }
	// Get the material at position. Requires read lock
	FORCEINLINE FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelMaterial>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelMaterial GetMaterial(const FIntVector& P      , int32 LOD) const { return Get<FVoxelMaterial>(P,       LOD); }
	
	// Set value at position. Requires write lock
	FORCEINLINE void SetValue(int32 X, int32 Y, int32 Z, FVoxelValue Value) { Set<FVoxelValue>(X, Y, Z, Value); }
	FORCEINLINE void SetValue(const FIntVector& P      , FVoxelValue Value) { Set<FVoxelValue>(P, Value); }
	// Set material at position. Requires write lock
	FORCEINLINE void SetMaterial(int32 X, int32 Y, int32 Z, FVoxelMaterial Material) { Set<FVoxelMaterial>(X, Y, Z, Material); }
	FORCEINLINE void SetMaterial(const FIntVector& P      , FVoxelMaterial Material) { Set<FVoxelMaterial>(P,       Material); }

public:
	/**
	 * Load/Save
	 */

	// Get a save of this world. No lock required
	void GetSave(FVoxelUncompressedWorldSaveImpl& OutSave);

	/**
	 * Load this world from save. No lock required
	 * @param	Save						Save to load from
	 * @param	OutBoundsToUpdate			The modified bounds
	 * @return true if loaded successfully, false if the world is corrupted and must not be saved again
	 */
	bool LoadFromSave(const AVoxelWorld* VoxelWorld, const FVoxelUncompressedWorldSaveImpl& Save, TArray<FIntBox>& OutBoundsToUpdate);


public:
	/**
	 * Undo/Redo
	 */

	// Undo one frame and add it to the redo stack. Current frame must be empty. No lock required
	bool Undo(TArray<FIntBox>& OutBoundsToUpdate);
	// Redo one frame and add it to the undo stack. Current frame must be empty. No lock required
	bool Redo(TArray<FIntBox>& OutBoundsToUpdate);
	// Clear all the frames. No lock required
	void ClearFrames();
	// Add the current frame to the undo stack. Clear the redo stack. No lock required. Bounds: must contain all the edits since last SaveFrame
	void SaveFrame(const FIntBox& Bounds);
	// Check that the current frame is empty (safe to call Undo/Redo). No lock required
	bool IsCurrentFrameEmpty();
	// Get the history position. No lock required
	inline int32 GetHistoryPosition() const { return HistoryPosition; }
	// Get the max history position, ie HistoryPosition + redo frames. No lock required
	inline int32 GetMaxHistoryPosition() const { return MaxHistoryPosition; }

	// Dirty state: can use that to track if the data is dirty
	// MarkAsDirty is called on Undo, Redo, SaveFrame and ClearData
	FORCEINLINE void MarkAsDirty() { bIsDirty = true; }
	FORCEINLINE void ClearDirtyFlag() { bIsDirty = false; }
	FORCEINLINE bool IsDirty() const { return bIsDirty; }

	// Each save frame call gets assigned a unique ID, can be used to track the state of the world
	// Will always be != 0
	FORCEINLINE uint64 GetCurrentFrameUniqueId() const { return CurrentFrameUniqueId; }
	
private:
	int32 HistoryPosition = 0;
	int32 MaxHistoryPosition = 0;
	
	TArray<FIntBox> UndoFramesBounds;
	TArray<FIntBox> RedoFramesBounds;
	
	// Used to clear redo stacks on SaveFrame without iterating the entire octree
	// Stack: added when undoing, poping when redoing
	TArray<TArray<FVoxelDataOctreeLeaf*>> LeavesWithRedoStackStack;

	// Each save frame is assigned a unique ID
	uint64 FrameUniqueIdCounter = 2;
	uint64 CurrentFrameUniqueId = 1;
	TArray<uint64> UndoUniqueIds;
	TArray<uint64> RedoUniqueIds;
	
	bool bIsDirty = false;

public:
	/**
	 * Placeable items
	 */

	// Add a FVoxelPlaceableItem to the world. Requires write lock on the item bounds
	template<typename T, typename... TArgs>
	inline TVoxelWeakPtr<T> AddItem(TArgs&&... Args)
	{
		auto Item = MakeVoxelShared<T>(Forward<TArgs>(Args)...);
		AddItem(Item, ERecordInHistory::Yes);
		return Item;
	}
	// Requires write lock on item bounds
	template<typename T>
	inline bool RemoveItem(TVoxelWeakPtr<T>& Item, bool bResetOverlappingChunksData, FString& OutError)
	{
		const auto ItemPtr = Item.Pin();
		if (ItemPtr.IsValid())
		{
			const bool bSuccess = RemoveItem(ItemPtr.Get(), ERecordInHistory::Yes, bResetOverlappingChunksData, OutError);
			Item.Reset();
			return bSuccess;
		}
		else
		{
			OutError = TEXT("Invalid item, or the item was already deleted");
			return false;
		}
	}

private:
	enum class ERecordInHistory
	{
		Yes,
		No
	};
	
	void AddItem(
		const TVoxelSharedRef<FVoxelPlaceableItem>& Item, 
		ERecordInHistory RecordInHistory, 
		bool bDoNotModifyExistingDataChunks = false);
	bool RemoveItem(FVoxelPlaceableItem* Item, ERecordInHistory RecordInHistory, bool bResetOverlappingChunksData, FString& OutError);
	
	struct FItemFrame
	{
		int32 HistoryPosition = -1;
		TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> AddedItems;
		TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> RemovedItems;

		inline bool IsEmpty() const { return AddedItems.Num() == 0 && RemovedItems.Num() == 0; }
	};

	FCriticalSection ItemsSection;
	TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> Items;
	TArray<int32> FreeItems;
	TUniquePtr<FItemFrame> ItemFrame = MakeUnique<FItemFrame>();
	TArray<TUniquePtr<FItemFrame>> ItemUndoFrames;
	TArray<TUniquePtr<FItemFrame>> ItemRedoFrames;
};

namespace FVoxelDataUtilities
{
	VOXEL_API void AddAssetItemDataToLeaf(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const FVoxelTransformableWorldGeneratorInstance& WorldGenerator, 
		const FTransform& LocalToWorld,
		bool bModifyValues,
		bool bModifyMaterials);
}

template<EVoxelLockType LockType>
class TVoxelScopeLock
{
public:
	using TData = typename TChooseClass<LockType == EVoxelLockType::Read, const FVoxelData, FVoxelData>::Result;
	
	TVoxelScopeLock(TData& InData, const FIntBox& Bounds, const FName& Name, bool bCondition = true)
		: Data(InData)
	{
		if (bCondition)
		{
			LockInfo = Data.Lock(LockType, Bounds, Name);
		}
	}
	~TVoxelScopeLock()
	{
		if (LockInfo.IsValid())
		{
			Unlock();
		}
	}

	void Unlock()
	{
		check(LockInfo.IsValid());
		Data.Unlock(MoveTemp(LockInfo));
	}

private:
	const FVoxelData& Data;
	TUniquePtr<FVoxelDataLockInfo> LockInfo;
};

class FVoxelReadScopeLock : public TVoxelScopeLock<EVoxelLockType::Read>
{
	using TVoxelScopeLock<EVoxelLockType::Read>::TVoxelScopeLock;
};
class FVoxelWriteScopeLock : public TVoxelScopeLock<EVoxelLockType::Write>
{
	using TVoxelScopeLock<EVoxelLockType::Write>::TVoxelScopeLock;
};

// Read lock that can be promoted to a write lock
class FVoxelPromotableReadScopeLock
{
public:
	FVoxelPromotableReadScopeLock(FVoxelData& Data, const FIntBox& Bounds, const FName& Name)
		: Data(Data)
		, Bounds(Bounds)
		, Name(Name)
	{
		LockInfo = Data.Lock(EVoxelLockType::Read, Bounds, Name);
	}
	~FVoxelPromotableReadScopeLock()
	{
		Data.Unlock(MoveTemp(LockInfo));
	}

	FORCEINLINE bool IsPromoted() const
	{
		return bPromoted;
	}
	void Promote()
	{
		check(!bPromoted);
		bPromoted = true;

		Data.Unlock(MoveTemp(LockInfo));
		LockInfo = Data.Lock(EVoxelLockType::Write, Bounds, Name);
	}

private:
	const FVoxelData& Data;
	const FIntBox Bounds;
	const FName Name;

	bool bPromoted = false;
	TUniquePtr<FVoxelDataLockInfo> LockInfo;
};