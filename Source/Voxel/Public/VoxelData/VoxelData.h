// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelSharedMutex.h"
#include "VoxelData/IVoxelData.h"
#include "HAL/ConsoleManager.h"

class AVoxelWorld;
class FVoxelData;
class FVoxelDataLockInfo;
class FVoxelDataOctreeBase;
class FVoxelDataOctreeLeaf;
class FVoxelDataOctreeParent;
class FVoxelGeneratorInstance;
class FVoxelTransformableGeneratorInstance;

struct FVoxelDataItem;
struct FVoxelAssetItem;
struct FVoxelObjectArchiveEntry;
struct FVoxelDisableEditsBoxItem;
struct FVoxelPlaceableItemLoadInfo;
struct FVoxelUncompressedWorldSaveImpl;

template<typename T>
struct TVoxelRange;
template<typename T>
class TVoxelQueryZone;
template<typename T>
struct TVoxelChunkDiff;

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Voxel Asset Items"), STAT_NumVoxelAssetItems, STATGROUP_VoxelCounters, VOXEL_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Voxel Disable Edits Items"), STAT_NumVoxelDisableEditsItems, STATGROUP_VoxelCounters, VOXEL_API);
DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num Voxel Data Items"), STAT_NumVoxelDataItems, STATGROUP_VoxelCounters, VOXEL_API);

extern VOXEL_API TAutoConsoleVariable<int32> CVarMaxPlaceableItemsPerOctree;
extern VOXEL_API TAutoConsoleVariable<int32> CVarStoreSpecialValueForGeneratorValuesInSaves;

// Turns off some expensive compression settings that aren't needed if you just want to save, recreate world, load
// TODO REMOVE AND MAKE Save/Load param
struct FVoxelScopedFastSaveLoad
{
	// No need to diff against generator as it's very slow
	const int32 DiffGenerator = CVarStoreSpecialValueForGeneratorValuesInSaves.GetValueOnGameThread();

	FVoxelScopedFastSaveLoad()
	{
		CVarStoreSpecialValueForGeneratorValuesInSaves->Set(0);
	}
	~FVoxelScopedFastSaveLoad()
	{
		CVarStoreSpecialValueForGeneratorValuesInSaves->Set(DiffGenerator);
	}
};

template<typename T>
class TVoxelDataItemWrapper
{
public:
	T Item;

private:
	mutable int32 Index = -1;
	TVoxelWeakPtr<FVoxelData> Data;
	
	friend class FVoxelData;
};

struct VOXEL_API FVoxelDataSettings
{
	const int32 Depth;
	const FVoxelIntBox WorldBounds;
	const TVoxelSharedRef<FVoxelGeneratorInstance> Generator;
	const bool bEnableMultiplayer;
	const bool bEnableUndoRedo;

	FVoxelDataSettings(const AVoxelWorld* World, EVoxelPlayType PlayType);
	FVoxelDataSettings(
		int32 Depth,
		const TVoxelSharedRef<FVoxelGeneratorInstance>& Generator,
		bool bEnableMultiplayer,
		bool bEnableUndoRedo);
	FVoxelDataSettings(
		const FVoxelIntBox& WorldBounds,
		const TVoxelSharedRef<FVoxelGeneratorInstance>& Generator,
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
	// Clone without keeping the voxel data
	TVoxelSharedRef<FVoxelData> Clone() const;
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
	FVoxelDataOctreeBase& GetOctree() const;

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
	
	template<typename T>
	FORCEINLINE void ClampToWorld(T& X, T& Y, T& Z) const
	{
		WorldBounds.Clamp(X, Y, Z);
		ensureVoxelSlow(IsInWorld(X, Y, Z));
	}
	template<typename T>
	FORCEINLINE T ClampToWorld(const T& P) const
	{
		return WorldBounds.Clamp(P);
	}

public:
	/**
	 * Lock the bounds
	 * @param	LockType			Read or write lock
	 * @param	Bounds				Bounds to lock
	 * @param	Name				The name of the task locking these bounds, for debug
	 */
	TUniquePtr<FVoxelDataLockInfo> Lock(EVoxelLockType LockType, const FVoxelIntBox& Bounds, FName Name) const;

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
	void ClearOctreeData(TArray<FVoxelIntBox>& OutBoundsToUpdate);

	// Requires write lock
	template<typename T>
	void CacheBounds(const FVoxelIntBox& Bounds, bool bMultiThreaded);
	
	// Requires write lock
	template<typename T>
	void ClearCacheInBounds(const FVoxelIntBox& Bounds);
	
	// Requires write lock
	template<typename T>
	void CheckIsSingle(const FVoxelIntBox& Bounds);

	// Get the data in zone. Requires read lock
	template<typename T>
	void Get(TVoxelQueryZone<T>& QueryZone, int32 LOD) const;
	
	template<typename T>
	TArray<T> Get(const FVoxelIntBox& Bounds) const;

	// Will always use 8 threads
	template<typename T>
	TArray<T> ParallelGet(const FVoxelIntBox& Bounds, bool bForceSingleThread = false) const;

	TArray<FVoxelValue> GetValues(const FVoxelIntBox& Bounds) const
	{
		return Get<FVoxelValue>(Bounds);
	}
	TArray<FVoxelMaterial> GetMaterials(const FVoxelIntBox& Bounds) const
	{
		return Get<FVoxelMaterial>(Bounds);
	}

	// Requires read lock
	TVoxelRange<FVoxelValue> GetValueRange(const FVoxelIntBox& Bounds, int32 LOD) const;

	bool IsEmpty(const FVoxelIntBox& Bounds, int32 LOD) const;

	template<typename T>
	T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;

	template<typename T, typename U>
	FORCEINLINE T GetCustomOutput(T DefaultValue, FName Name, const U& P, int32 LOD) const
	{
		return GetCustomOutput<T>(DefaultValue, Name, P.X, P.Y, P.Z, LOD);
	}
	
	TVoxelRange<v_flt> GetCustomOutputRange(TVoxelRange<v_flt> DefaultValue, FName Name, const FVoxelIntBox& Bounds, int32 LOD) const;
	
public:
	template<typename ...TArgs, typename F>
	void Set(const FVoxelIntBox& Bounds, F Apply);

	template<typename ...TArgs, typename F>
	void ParallelSet(const FVoxelIntBox& Bounds, F Apply, bool bForceSingleThread = false);

public:
	/**
	 * Getters/Setters
	 */
	// Set value or material at position depending on template argument (FVoxelValue or FVoxelMaterial)
	template<typename T>
	void Set(int32 X, int32 Y, int32 Z, const T& Value);

	template<typename T>
	FORCEINLINE void Set(const FIntVector& P, const T& Value)
	{
		Set<T>(P.X, P.Y, P.Z, Value);
	}

	template<typename T>
	T Get(int32 X, int32 Y, int32 Z, int32 LOD) const;

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
	void GetSave(FVoxelUncompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects);

	/**
	 * Load this world from save. No lock required
	 * @param	Save						Save to load from
	 * @param	LoadInfo					Used to load placeable items. Can use {}
	 * @param	OutBoundsToUpdate			The modified bounds
	 * @return true if loaded successfully, false if the world is corrupted and must not be saved again
	 */
	bool LoadFromSave(const FVoxelUncompressedWorldSaveImpl& Save, const FVoxelPlaceableItemLoadInfo& LoadInfo, TArray<FVoxelIntBox>* OutBoundsToUpdate = nullptr);


public:
	/**
	 * Undo/Redo
	 */

	// Undo one frame and add it to the redo stack. Current frame must be empty. No lock required
	bool Undo(TArray<FVoxelIntBox>& OutBoundsToUpdate);
	// Redo one frame and add it to the undo stack. Current frame must be empty. No lock required
	bool Redo(TArray<FVoxelIntBox>& OutBoundsToUpdate);
	// Clear all the frames. No lock required
	void ClearFrames();
	// Add the current frame to the undo stack. Clear the redo stack. No lock required. Bounds: must contain all the edits since last SaveFrame
	void SaveFrame(const FVoxelIntBox& Bounds);
	// Check that the current frame is empty (safe to call Undo/Redo). No lock required
	bool IsCurrentFrameEmpty();
	// Get the history position. No lock required
	inline int32 GetHistoryPosition() const { return UndoRedo.HistoryPosition; }
	// Get the max history position, ie HistoryPosition + redo frames. No lock required
	inline int32 GetMaxHistoryPosition() const { return UndoRedo.MaxHistoryPosition; }

	// Dirty state: can use that to track if the data is dirty
	// MarkAsDirty is called on Undo, Redo, SaveFrame and ClearData
	FORCEINLINE void MarkAsDirty() { bIsDirty = true; }
	FORCEINLINE void ClearDirtyFlag() { bIsDirty = false; }
	FORCEINLINE bool IsDirty() const { return bIsDirty; }

	// Each save frame call gets assigned a unique ID, can be used to track the state of the world
	// Will always be != 0
	FORCEINLINE uint64 GetCurrentFrameUniqueId() const { return UndoRedo.CurrentFrameUniqueId; }
	
private:
	struct FUndoRedo
	{
		int32 HistoryPosition = 0;
		int32 MaxHistoryPosition = 0;
		
		TArray<FVoxelIntBox> UndoFramesBounds;
		TArray<FVoxelIntBox> RedoFramesBounds;
		
		// Used to clear redo stacks on SaveFrame without iterating the entire octree
		// Stack: added when undoing, poping when redoing
		TArray<TArray<FVoxelDataOctreeLeaf*>> LeavesWithRedoStackStack;

		// Each save frame is assigned a unique ID
		uint64 FrameUniqueIdCounter = 2;
		uint64 CurrentFrameUniqueId = 1;
		TArray<uint64> UndoUniqueIds;
		TArray<uint64> RedoUniqueIds;
	};
	FUndoRedo UndoRedo;
	bool bIsDirty = false;

public:
	/**
	 * Placeable items
	 */

	/** Add a FVoxelPlaceableItem to the world. Requires write lock on the item bounds
	 *	@param	Args							Passed to the constructor of T
	 *	@param	bDoNotModifyExistingDataChunks	Used when loading from a save
	 */
	template<typename T, bool bDoNotModifyExistingDataChunks = false, typename... TArgs>
	TVoxelWeakPtr<const TVoxelDataItemWrapper<T>> AddItem(TArgs&&... Args);

	// Requires write lock on item bounds
	template<typename T>
	bool RemoveItem(TVoxelWeakPtr<const TVoxelDataItemWrapper<T>>& Item, FString& OutError);

private:
	template<typename T>
	struct TItemData
	{
		FCriticalSection Section;
		TArray<TVoxelSharedPtr<const TVoxelDataItemWrapper<T>>> Items;
	};

	TItemData<FVoxelAssetItem> AssetItemsData;
	TItemData<FVoxelDisableEditsBoxItem> DisableEditsItemsData;
	TItemData<FVoxelDataItem> DataItemsData;
	// When adding a new item type also add it to ClearData, AddItem & RemoveItem, ApplyToAllItems, NumItems, NeedToSubdivide
	
	template<typename T>
	TItemData<T>& GetItemsData();
};

namespace FVoxelDataUtilities
{
	// This will NOT add the item to the item holder
	VOXEL_API void AddAssetItemToLeafData(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const FVoxelTransformableGeneratorInstance& Generator, 
		const FVoxelIntBox& Bounds,
		const FTransform& LocalToWorld,
		bool bModifyValues,
		bool bModifyMaterials);

	// Will update all the values that were the same as the old generator to the new generator values
	template<typename T, typename TApplyOld, typename TApplyNew>
	void MigrateLeafDataToNewGenerator(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const FVoxelIntBox& BoundsToMigrate,
		TApplyOld ApplyOldGenerator,
		TApplyNew ApplyNewGenerator);
	
	// This will NOT add the item to the item holder, but will assume it has already been added
	template<typename T, typename TItem>
	void AddItemToLeafData(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const TItem& Item);
	
	// This will NOT remove the item from the item holder, but will assume it has already been removed
	template<typename T, typename TItem>
	void RemoveItemFromLeafData(
		const FVoxelData& Data,
		FVoxelDataOctreeLeaf& Leaf,
		const TItem& Item);
}