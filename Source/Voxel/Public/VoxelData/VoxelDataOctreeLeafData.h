// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelFoliage.h"

DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Data Octree Values Memory"), STAT_VoxelDataOctreeValuesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Data Octree Materials Memory"), STAT_VoxelDataOctreeMaterialsMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Data Octree Foliage Memory"), STAT_VoxelDataOctreeFoliageMemory, STATGROUP_VoxelMemory, VOXEL_API);

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
		ClearData();
	}

	TVoxelDataOctreeLeafData(const TVoxelDataOctreeLeafData&) = delete;
	TVoxelDataOctreeLeafData(TVoxelDataOctreeLeafData&&) = delete;
	TVoxelDataOctreeLeafData& operator=(const TVoxelDataOctreeLeafData&) = delete;

public:
	void ClearData()
	{
		CheckState();
		if (DataPtr)
		{
			Deallocate();
		}
		bIsSingleValue = false;
		bDirty = false;
		CheckState();
	}
	
	void CreateDataPtr()
	{
		CheckState();
		Allocate();
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
	void SetDirty()
	{
		CheckState();
		bDirty = true;
		CheckState();
	}

	void ExpandSingleValue()
	{
		CheckState();
		check(bIsSingleValue);

		bIsSingleValue = false;
		Allocate();
		for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			DataPtr[Index] = SingleValue;
		}
		CheckState();
	}
	void TryCompressToSingleValue()
	{
		CheckState();
		check(!bIsSingleValue);
		
		if (!DataPtr) return;

		SingleValue = DataPtr[0];
		for (int32 Index = 1; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			if (DataPtr[Index] != SingleValue) return;
		}

		Deallocate();
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
	
	void Allocate()
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(!DataPtr && !bIsSingleValue);
		// TODO PERF: Custom allocator?
		DataPtr = static_cast<T*>(FMemory::Malloc(VOXELS_PER_DATA_CHUNK * sizeof(T)));
		if (TIsSame<T, FVoxelValue   >::Value) { INC_MEMORY_STAT_BY(STAT_VoxelDataOctreeValuesMemory   , VOXELS_PER_DATA_CHUNK * sizeof(T)); }
		if (TIsSame<T, FVoxelMaterial>::Value) { INC_MEMORY_STAT_BY(STAT_VoxelDataOctreeMaterialsMemory, VOXELS_PER_DATA_CHUNK * sizeof(T)); }
		if (TIsSame<T, FVoxelFoliage >::Value) { INC_MEMORY_STAT_BY(STAT_VoxelDataOctreeFoliageMemory  , VOXELS_PER_DATA_CHUNK * sizeof(T)); }
	}
	void Deallocate()
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(DataPtr);
		FMemory::Free(DataPtr);
		DataPtr = nullptr;
		if (TIsSame<T, FVoxelValue   >::Value) { DEC_MEMORY_STAT_BY(STAT_VoxelDataOctreeValuesMemory   , VOXELS_PER_DATA_CHUNK * sizeof(T)); }
		if (TIsSame<T, FVoxelMaterial>::Value) { DEC_MEMORY_STAT_BY(STAT_VoxelDataOctreeMaterialsMemory, VOXELS_PER_DATA_CHUNK * sizeof(T)); }
		if (TIsSame<T, FVoxelFoliage >::Value) { DEC_MEMORY_STAT_BY(STAT_VoxelDataOctreeFoliageMemory  , VOXELS_PER_DATA_CHUNK * sizeof(T)); }
	}
};