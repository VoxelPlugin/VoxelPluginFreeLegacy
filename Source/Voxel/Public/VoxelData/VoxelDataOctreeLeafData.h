// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelFoliage.h"
#include "VoxelMiscUtilities.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Dirty Values Memory"), STAT_VoxelDataOctreeDirtyValuesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Dirty Materials Memory"), STAT_VoxelDataOctreeDirtyMaterialsMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Dirty Foliage Memory"), STAT_VoxelDataOctreeDirtyFoliageMemory, STATGROUP_VoxelMemory, VOXEL_API);

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cached Values Memory"), STAT_VoxelDataOctreeCachedValuesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cached Materials Memory"), STAT_VoxelDataOctreeCachedMaterialsMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cached Foliage Memory"), STAT_VoxelDataOctreeCachedFoliageMemory, STATGROUP_VoxelMemory, VOXEL_API);

class IVoxelDataOctreeMemory
{
public:
	struct FDataOctreeMemory
	{
		uint8 PadToAvoidContention0[PLATFORM_CACHE_LINE_SIZE];
		FThreadSafeCounter64 Values;
		uint8 PadToAvoidContention1[PLATFORM_CACHE_LINE_SIZE];
		FThreadSafeCounter64 Materials;
		uint8 PadToAvoidContention2[PLATFORM_CACHE_LINE_SIZE];
		FThreadSafeCounter64 Foliage;
		uint8 PadToAvoidContention3[PLATFORM_CACHE_LINE_SIZE];
	};

	const FDataOctreeMemory& GetCachedMemory() const { return CachedMemory; }
	const FDataOctreeMemory& GetDirtyMemory() const { return DirtyMemory; }
	
private:
	mutable FDataOctreeMemory CachedMemory{};
	mutable FDataOctreeMemory DirtyMemory{};
	
	template<typename T>
	friend class TVoxelDataOctreeLeafData;
};

template<typename T>
class TVoxelDataOctreeLeafData
{
	static_assert(
		TIsSame<T, FVoxelValue>::Value || 
		TIsSame<T, FVoxelMaterial>::Value || 
		TIsSame<T, FVoxelFoliage>::Value ||
		TIsSame<T, const FVoxelValue>::Value || 
		TIsSame<T, const FVoxelMaterial>::Value || 
		TIsSame<T, const FVoxelFoliage>::Value, "");
public:
	TVoxelDataOctreeLeafData() = default;
	~TVoxelDataOctreeLeafData()
	{
		if (!ensureVoxelSlow(!DataPtr))
		{
			IVoxelDataOctreeMemory Memory;
			ClearData(Memory);
		}
	}

	TVoxelDataOctreeLeafData(const TVoxelDataOctreeLeafData&) = delete;
	TVoxelDataOctreeLeafData(TVoxelDataOctreeLeafData&&) = delete;
	TVoxelDataOctreeLeafData& operator=(const TVoxelDataOctreeLeafData&) = delete;

public:
	void ClearData(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		SetDirtyInternal(false, Memory);
		if (DataPtr)
		{
			Deallocate(Memory);
		}
		bIsSingleValue = false;
		CheckState();
	}
	
	void CreateDataPtr(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		Allocate(Memory);
		CheckState();
	}
	void SetSingleValue(T InSingleValue)
	{
		CheckState();
		check(!DataPtr && !bIsSingleValue);
		bIsSingleValue = true;
		SingleValue = InSingleValue;
		CheckState();
	}
	void SetDirty(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		SetDirtyInternal(true, Memory);
		CheckState();
	}
	void SetDirtyInternal(bool bNewDirty, const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		if (bDirty != bNewDirty)
		{
			if (DataPtr)
			{
				DecreaseMemoryUsage(bDirty, Memory);
			}
			bDirty = bNewDirty;
			if (DataPtr)
			{
				IncreaseMemoryUsage(bDirty, Memory);
			}
		}
		CheckState();
	}

	void ExpandSingleValue(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		check(bIsSingleValue);

		bIsSingleValue = false;
		Allocate(Memory);
		for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			DataPtr[Index] = SingleValue;
		}
		CheckState();
	}
	void TryCompressToSingleValue(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		check(!bIsSingleValue);
		
		if (!DataPtr) return;

		SingleValue = DataPtr[0];
		for (int32 Index = 1; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			if (DataPtr[Index] != SingleValue) return;
		}

		Deallocate(Memory);
		bIsSingleValue = true;
		
		CheckState();
	}

public:
	FORCEINLINE bool IsDirty() const
	{
		return bDirty;
	}
	FORCEINLINE T* RESTRICT GetDataPtr() const
	{
		return DataPtr;
	}
	FORCEINLINE bool IsSingleValue() const
	{
		return bIsSingleValue;
	}
	FORCEINLINE T GetSingleValue() const
	{
		checkVoxelSlow(IsSingleValue());
		return SingleValue;
	}
	FORCEINLINE void CheckState() const
	{
		checkVoxelSlow(!(DataPtr && bIsSingleValue));
		checkVoxelSlow(!bDirty || DataPtr || bIsSingleValue);
	}

public:
	// Implicit cast to const version
	FORCEINLINE operator TVoxelDataOctreeLeafData<const T>& ()
	{
		return reinterpret_cast<TVoxelDataOctreeLeafData<const T>&>(*this);
	}
	FORCEINLINE operator const TVoxelDataOctreeLeafData<const T>& () const
	{
		return reinterpret_cast<const TVoxelDataOctreeLeafData<const T>&>(*this);
	}

private:
	T* RESTRICT DataPtr = nullptr;
	bool bDirty = false;
	bool bIsSingleValue = false;
	T SingleValue;

	static constexpr int32 MemorySize = VOXELS_PER_DATA_CHUNK * sizeof(T);
	
	void Allocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(!DataPtr && !bIsSingleValue);
		// TODO PERF: Custom allocator?
		DataPtr = static_cast<T*>(FMemory::Malloc(MemorySize));
		
		IncreaseMemoryUsage(bDirty, Memory);
	}
	void Deallocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(DataPtr);
		FMemory::Free(DataPtr);
		DataPtr = nullptr;

		DecreaseMemoryUsage(bDirty, Memory);
	}
	
	static void IncreaseMemoryUsage(bool bInDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bInDirty)
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).Add(MemorySize);
			
			if (TIsSame<T, FVoxelValue   >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyMaterialsMemory, MemorySize); }
			if (TIsSame<T, FVoxelFoliage >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyFoliageMemory  , MemorySize); }
		}
		else
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).Add(MemorySize);

			if (TIsSame<T, FVoxelValue   >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedMaterialsMemory, MemorySize); }
			if (TIsSame<T, FVoxelFoliage >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedFoliageMemory  , MemorySize); }
		}
	}
	static void DecreaseMemoryUsage(bool bInDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bInDirty)
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).Subtract(MemorySize);
			ensureVoxelSlowNoSideEffects(FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).GetValue() >= 0);
			
			if (TIsSame<T, FVoxelValue   >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyMaterialsMemory, MemorySize); }
			if (TIsSame<T, FVoxelFoliage >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyFoliageMemory  , MemorySize); }
		}
		else
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).Subtract(MemorySize);
			ensureVoxelSlowNoSideEffects(FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).GetValue() >= 0);
			
			if (TIsSame<T, FVoxelValue   >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedMaterialsMemory, MemorySize); }
			if (TIsSame<T, FVoxelFoliage >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedFoliageMemory  , MemorySize); }
		}
	}
};