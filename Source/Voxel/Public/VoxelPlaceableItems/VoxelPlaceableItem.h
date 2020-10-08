// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"

class AVoxelWorld;
class FVoxelObjectArchive;
class FVoxelGeneratorInstance;
class FVoxelTransformableGeneratorInstance;

struct FVoxelGeneratorInit;
struct FVoxelObjectArchiveEntry;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct FVoxelAssetItem
{
	TVoxelSharedPtr<FVoxelTransformableGeneratorInstance> Generator;
	FVoxelIntBox Bounds;
	FTransform LocalToWorld;
	// Assets are sorted by priority
	int32 Priority;

	static void Sort(TArray<const FVoxelAssetItem*>& Array)
	{
		Array.Sort([](const FVoxelAssetItem& A, const FVoxelAssetItem& B) { return A.Priority < B.Priority; });
	}
};

struct FVoxelDisableEditsBoxItem
{
	FVoxelIntBox Bounds;

	static void Sort(TArray<const FVoxelDisableEditsBoxItem*>& Array) {}
};

struct FVoxelDataItem
{
	TVoxelSharedPtr<FVoxelGeneratorInstance> Generator;
	FVoxelIntBox Bounds;
	TArray<v_flt> Data;
	uint32 Mask = 0;

	static void Sort(TArray<const FVoxelDataItem*>& Array) {}
};

#define FOREACH_VOXEL_ASSET_ITEM(Macro) \
	Macro(AssetItem) \
	Macro(DisableEditsBoxItem) \
	Macro(DataItem)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace FVoxelPlaceableItemVersion
{
	enum Type : int32
	{
		FirstVersion,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
}

struct FVoxelPlaceableItemLoadInfo
{
	const FVoxelGeneratorInit* GeneratorInit = nullptr;
	const TArray<FVoxelObjectArchiveEntry>* Objects = nullptr;
};

namespace FVoxelPlaceableItemsUtilities
{
	VOXEL_API void SerializeItems(
		FVoxelObjectArchive& Ar,
		const FVoxelPlaceableItemLoadInfo& LoadInfo,
		TArray<FVoxelAssetItem>& AssetItems);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Placeable Items Pointers Memory"), STAT_VoxelPlaceableItemsPointers, STATGROUP_VoxelMemory, VOXEL_API);

#define Macro(X) DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("Num " #X " Pointers"), STAT_Num## X ## Pointers, STATGROUP_VoxelCounters, VOXEL_API);
FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro

class FVoxelPlaceableItemHolder
{
public:	
	FVoxelPlaceableItemHolder() = default;
	~FVoxelPlaceableItemHolder()
	{
#define Macro(X) DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPlaceableItemsPointers, X.GetAllocatedSize()); DEC_DWORD_STAT_BY(STAT_Num ## X ## Pointers, X.Num()); X.Reset();
		FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
	}

private:
#define Macro(X) TArray<const FVoxel ## X *> X;
	FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro

public:
	template<typename T>
	void ApplyToAllItems(T Lambda)
	{
#define Macro(X) for (auto* Item : X) { Lambda(*Item); }
		FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
	}

public:
	int32 NumItems() const
	{
		int32 Num = 0;
#define Macro(X) Num += X.Num();
		FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
		return Num;
	}
	bool NeedToSubdivide(int32 Threshold) const
	{
		bool bValue = false;
#define Macro(X) bValue |= X.Num() > Threshold;
		FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
		return bValue;
	}

public:
#define Macro(X) const TArray<const FVoxel ## X*>& Get ## X ## s() const { return X; }
	FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
	
#define Macro(X) \
	void AddItem(const FVoxel ## X & Item) \
	{ \
		INC_DWORD_STAT(STAT_Num ## X ## Pointers); \
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPlaceableItemsPointers, X.GetAllocatedSize()); \
		\
		ensureVoxelSlowNoSideEffects(!X.Contains(&Item)); \
		X.Add(&Item); \
		FVoxel ## X :: Sort(X); \
		\
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPlaceableItemsPointers, X.GetAllocatedSize()); \
	}
	FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
	
#define Macro(X) \
	bool RemoveItem(const FVoxel ## X& Item) \
	{ \
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPlaceableItemsPointers, X.GetAllocatedSize()); \
		\
		const int32 NumRemoved = X.Remove(&Item); \
		ensureVoxelSlow(NumRemoved <= 1); \
		if (NumRemoved) \
		{ \
			DEC_DWORD_STAT(STAT_Num ## X ## Pointers); \
		} \
		\
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPlaceableItemsPointers, X.GetAllocatedSize()); \
		return NumRemoved != 0; \
	}
	FOREACH_VOXEL_ASSET_ITEM(Macro);
#undef Macro
};