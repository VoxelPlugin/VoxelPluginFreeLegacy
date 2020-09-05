// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelData/IVoxelData.h"
#include "VoxelUtilities/VoxelMiscUtilities.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Dirty Values Memory"), STAT_VoxelDataOctreeDirtyValuesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Dirty Materials Memory"), STAT_VoxelDataOctreeDirtyMaterialsMemory, STATGROUP_VoxelMemory, VOXEL_API);

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cached Values Memory"), STAT_VoxelDataOctreeCachedValuesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cached Materials Memory"), STAT_VoxelDataOctreeCachedMaterialsMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<typename T>
struct TVoxelDataOctreeLeafMemoryUsage
{
	static_assert(TIsSame<T, FVoxelValue>::Value || TIsSame<T, FVoxelMaterial>::Value, "");
	
	static void Increase(int32 MemorySize, bool bDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bDirty)
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).Add(MemorySize);
			
			if (TIsSame<T, FVoxelValue   >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyMaterialsMemory, MemorySize); }
		}
		else
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).Add(MemorySize);

			if (TIsSame<T, FVoxelValue   >::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedMaterialsMemory, MemorySize); }
		}
	}
	static void Decrease(int32 MemorySize, bool bDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bDirty)
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).Subtract(MemorySize);
			ensureVoxelSlowNoSideEffects(FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.DirtyMemory).GetValue() >= 0);
			
			if (TIsSame<T, FVoxelValue   >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeDirtyMaterialsMemory, MemorySize); }
		}
		else
		{
			FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).Subtract(MemorySize);
			ensureVoxelSlowNoSideEffects(FVoxelUtilities::TValuesMaterialsSelector<T>::Get(Memory.CachedMemory).GetValue() >= 0);
			
			if (TIsSame<T, FVoxelValue   >::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedValuesMemory   , MemorySize); }
			if (TIsSame<T, FVoxelMaterial>::Value) { DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataOctreeCachedMaterialsMemory, MemorySize); }
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
class TVoxelDataOctreeLeafData;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
class TVoxelDataOctreeLeafData<FVoxelValue>
{
	FVoxelValue* RESTRICT DataPtr = nullptr;
	FVoxelValue SingleValue;
	bool bIsSingleValue = false;
	bool bDirty = false;

	static constexpr int32 MemorySize = VOXELS_PER_DATA_CHUNK * sizeof(FVoxelValue);

	friend class FVoxelSaveBuilder;
	friend class FVoxelSaveLoader;
	
public:
	TVoxelDataOctreeLeafData() = default;
	~TVoxelDataOctreeLeafData()
	{
		if (!ensureVoxelSlow(!DataPtr))
		{
			ClearData(IVoxelDataOctreeMemory());
		}
	}

	UE_NONCOPYABLE(TVoxelDataOctreeLeafData);
	
public:
	FORCEINLINE bool IsDirty() const
	{
		return bDirty;
	}
	
	void SetIsDirty(bool bNewDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bDirty == bNewDirty)
		{
			return;
		}
	
		const bool bOldDirty = bDirty;
		bDirty = bNewDirty;

		// Update the memory usages according to the new dirty flags
		// This is so that the memory usage reported by the voxel world (eg to get the size of the map voxel data) is accurate
		// without including cached memory usage into dirty memory usage
		if (DataPtr)
		{
			TVoxelDataOctreeLeafMemoryUsage<FVoxelValue>::Decrease(MemorySize, bOldDirty, Memory);
			TVoxelDataOctreeLeafMemoryUsage<FVoxelValue>::Increase(MemorySize, bNewDirty, Memory);
		}
	}

public:
	void CreateData(const IVoxelDataOctreeMemory& Memory)
	{
		check(!HasData());
		CheckState();
		Allocate(Memory);
		CheckState();
	}
	template<typename TLambda>
	void CreateData(const IVoxelDataOctreeMemory& Memory, TLambda Init)
	{
		CreateData(Memory);
		check(DataPtr);
		Init(static_cast<FVoxelValue* RESTRICT>(DataPtr));
	}
	void CreateData(const IVoxelDataOctreeMemory& Memory, const TVoxelDataOctreeLeafData<FVoxelValue>& Source)
	{
		check(!HasData());
		CheckState();

		bIsSingleValue = Source.bIsSingleValue;
		if (Source.bIsSingleValue)
		{
			SingleValue = Source.SingleValue;
		}
		else
		{
			if (Source.DataPtr)
			{
				Allocate(Memory);
				FMemory::Memcpy(DataPtr, Source.DataPtr, MemorySize);
			}
		}
		CheckState();
	}
	
	void ClearData(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		SetIsDirty(false, Memory);
		if (DataPtr)
		{
			Deallocate(Memory);
		}
		bIsSingleValue = false;
		checkVoxelSlow(!HasData());
		CheckState();
	}

public:
	// Used to determine if it's worth compressing or clearing the cache
	FORCEINLINE bool HasAllocation() const
	{
		return DataPtr != nullptr;
	}
	FORCEINLINE bool HasData() const
	{
		return DataPtr || bIsSingleValue;
	}
	
public:
	void Compress(const IVoxelDataOctreeMemory& Memory)
	{
		if (!bIsSingleValue)
		{
			TryCompressToSingleValue(Memory);
		}
	}

public:
	FORCEINLINE FVoxelValue Get(int32 Index) const
	{
		checkVoxelSlow(HasData());
		CheckBounds(Index);
		if (bIsSingleValue)
		{
			return SingleValue;
		}
		else
		{
			checkVoxelSlow(DataPtr);
			return DataPtr[Index];
		}
	}

public:
	FORCEINLINE void PrepareForWrite(const IVoxelDataOctreeMemory& Memory)
	{
		checkVoxelSlow(HasData());
		CheckState();
		if (bIsSingleValue)
		{
			ExpandSingleValue(Memory);
		}
		CheckState();
	}
	FORCEINLINE FVoxelValue& GetRef(int32 Index)
	{
		checkVoxelSlow(DataPtr);
		checkVoxelSlow(HasData());
		CheckBounds(Index);
		return DataPtr[Index];
	}

public:
	FORCEINLINE void CopyTo(FVoxelValue* RESTRICT DestPtr) const
	{
		checkVoxelSlow(HasData());
		if (bIsSingleValue)
		{
			for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
			{
				DestPtr[Index] = SingleValue;
			}
		}
		else
		{
			FMemory::Memcpy(DestPtr, DataPtr, MemorySize);
		}
	}

public:
	FORCEINLINE bool IsSingleValue() const
	{
		return bIsSingleValue;
	}
	FORCEINLINE FVoxelValue GetSingleValue() const
	{
		checkVoxelSlow(IsSingleValue());
		return SingleValue;
	}
	
	void SetSingleValue(FVoxelValue InSingleValue)
	{
		CheckState();
		check(!DataPtr && !bIsSingleValue);
		bIsSingleValue = true;
		SingleValue = InSingleValue;
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
		
		if (!DataPtr)
		{
			return;
		}

		const FVoxelValue NewSingleValue = DataPtr[0];
		for (int32 Index = 1; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			if (DataPtr[Index] != NewSingleValue) return;
		}

		Deallocate(Memory);
		SingleValue = NewSingleValue;
		bIsSingleValue = true;
		
		CheckState();
	}
	
private:
	FORCEINLINE void CheckState() const
	{
		checkVoxelSlow(!(DataPtr && bIsSingleValue));
		checkVoxelSlow(!bDirty || HasData());
	}
	FORCEINLINE static void CheckBounds(int32 Index)
	{
		checkVoxelSlow(0 <= Index && Index < VOXELS_PER_DATA_CHUNK);
	}
	
private:
	void Allocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(!DataPtr && !bIsSingleValue);
		DataPtr = static_cast<FVoxelValue*>(FMemory::Malloc(MemorySize));
		
		TVoxelDataOctreeLeafMemoryUsage<FVoxelValue>::Increase(MemorySize, bDirty, Memory);
	}
	void Deallocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(DataPtr);
		FMemory::Free(DataPtr);
		DataPtr = nullptr;
		
		TVoxelDataOctreeLeafMemoryUsage<FVoxelValue>::Decrease(MemorySize, bDirty, Memory);
	}
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
class TVoxelDataOctreeLeafData<FVoxelMaterial>
{
	static constexpr int32 NumChannels = FVoxelMaterial::NumChannels;

	TVoxelStaticArray<uint8*, NumChannels> Channels_DataPtr{ ForceInit };
	TVoxelStaticArray<uint8, NumChannels> Channels_SingleValue{ ForceInit };
	
	FVoxelMaterial* RESTRICT Main_DataPtr = nullptr;
	// If set, implies the data stored in Channels is valid
	// If Channels_DataPtr[I] is null, then Channels_SingleValue[I] is valid
	// Data in Channels is assumed constant: compression won't try to compress it again
	bool bUseChannels = false;
	bool bDirty = false;

	friend class FVoxelSaveBuilder;
	friend class FVoxelSaveLoader;

public:
	TVoxelDataOctreeLeafData() = default;
	~TVoxelDataOctreeLeafData()
	{
		bool bClear = !ensureVoxelSlow(!Main_DataPtr);
		for (auto& DataPtr : Channels_DataPtr)
		{
			bClear |= !ensureVoxelSlow(!DataPtr);
		}

		if (bClear)
		{
			ClearData(IVoxelDataOctreeMemory());
		}
	}

	UE_NONCOPYABLE(TVoxelDataOctreeLeafData);

public:
	FORCEINLINE bool IsDirty() const
	{
		return bDirty;
	}
	
	void SetIsDirty(const bool bNewDirty, const IVoxelDataOctreeMemory& Memory)
	{
		if (bDirty == bNewDirty)
		{
			return;
		}
	
		const bool bOldDirty = bDirty;
		bDirty = bNewDirty;

		// Update the memory usages according to the new dirty flags
		// This is so that the memory usage reported by the voxel world (eg to get the size of the map voxel data) is accurate
		// without including cached memory usage into dirty memory usage
		if (bUseChannels)
		{
			for (auto& DataPtr : Channels_DataPtr)
			{
				if (DataPtr)
				{
					TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Decrease(Channels_MemorySize, bOldDirty, Memory);
					TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Increase(Channels_MemorySize, bNewDirty, Memory);
				}
			}
		}
		else
		{
			if (Main_DataPtr)
			{
				TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Decrease(Main_MemorySize, bOldDirty, Memory);
				TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Increase(Main_MemorySize, bNewDirty, Memory);
			}
		}
	}

public:
	void CreateData(const IVoxelDataOctreeMemory& Memory)
	{
		check(!HasData());
		CheckState();
		Main_Allocate(Memory);
		CheckState();
	}
	template<typename TLambda>
	void CreateData(const IVoxelDataOctreeMemory& Memory, TLambda Init)
	{
		CreateData(Memory);
		check(Main_DataPtr);
		Init(static_cast<FVoxelMaterial* RESTRICT>(Main_DataPtr));
	}
	void CreateData(const IVoxelDataOctreeMemory& Memory, const TVoxelDataOctreeLeafData<FVoxelMaterial>& Source)
	{
		check(!HasData());
		CheckState();
		bUseChannels = Source.bUseChannels;
		if (Source.bUseChannels)
		{
			for (int32 Channel = 0; Channel < NumChannels; Channel++)
			{
				auto* SourceDataPtr = Source.Channels_DataPtr[Channel];
				if (SourceDataPtr)
				{
					auto*& DataPtr = Channels_DataPtr[Channel];
					Channels_Allocate(DataPtr, Memory);

					FMemory::Memcpy(DataPtr, SourceDataPtr, Channels_MemorySize);
				}
			}
		}
		else
		{
			if (Source.Main_DataPtr)
			{
				FMemory::Memcpy(Main_DataPtr, Source.Main_DataPtr, Main_MemorySize);
			}
		}
		CheckState();
	}
	
	void ClearData(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		SetIsDirty(false, Memory);
		if (bUseChannels)
		{
			for (auto& DataPtr : Channels_DataPtr)
			{
				if (DataPtr)
				{
					Channels_Deallocate(DataPtr, Memory);
				}
			}
		}
		else
		{
			if (Main_DataPtr)
			{
				Main_Deallocate(Memory);
			}
		}
		bUseChannels = false;
		checkVoxelSlow(!HasData());
		CheckState();
	}

public:
	// Used to determine if it's worth compressing or clearing the cache
	FORCEINLINE bool HasAllocation() const
	{
		if (bUseChannels)
		{
			for (auto& DataPtr : Channels_DataPtr)
			{
				if (DataPtr)
				{
					return true;
				}
			}
			return false;
		}
		else
		{
			return Main_DataPtr != nullptr;
		}
	}
	FORCEINLINE bool HasData() const
	{
		return bUseChannels || Main_DataPtr;
	}
	
public:
	void Compress(const IVoxelDataOctreeMemory& Memory)
	{
		CheckState();
		
		if (bUseChannels || !Main_DataPtr)
		{
			return;
		}
		
		const FVoxelMaterial SingleMaterial = Main_DataPtr[0];

		static_assert(NumChannels < 31, "");
		constexpr uint32 DoNotCompressAnyChannel = (1 << NumChannels) - 1;

		uint32 DoNotCompressChannel = 0;

		// Iterate all the data, checking for constant channels
		for (int32 Index = 1; Index < VOXELS_PER_DATA_CHUNK; Index++)
		{
			for (int32 Channel = 0; Channel < NumChannels; Channel++)
			{
				const bool bDifferent = SingleMaterial.GetRaw(Channel) != Main_DataPtr[Index].GetRaw(Channel);
				DoNotCompressChannel |= (1 << Channel) * bDifferent;
			}

			if (DoNotCompressChannel == DoNotCompressAnyChannel)
			{
				// Fast path if all channels are different
				return;
			}
		}
		checkVoxelSlow(DoNotCompressChannel != DoNotCompressAnyChannel);

		// Create channels
		for (int32 Channel = 0; Channel < NumChannels; Channel++)
		{
			if (DoNotCompressChannel & (1 << Channel))
			{
				uint8* RESTRICT& DataPtr = Channels_DataPtr[Channel];
				Channels_Allocate(DataPtr, Memory);

				// Copy data from main
				for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
				{
					DataPtr[Index] = Main_DataPtr[Index].GetRaw(Channel);
				}
			}
			else
			{
				// Use the constant value we found
				Channels_SingleValue[Channel] = SingleMaterial.GetRaw(Channel);
			}
		}

		// Then delete main
		Main_Deallocate(Memory);
		bUseChannels = true;

		CheckState();
	}

public:
	FORCEINLINE FVoxelMaterial Get(int32 Index) const
	{
		checkVoxelSlow(HasData());
		CheckBounds(Index);

		if (bUseChannels)
		{
			return GetFromChannels(Index);
		}
		else
		{
			checkVoxelSlow(Main_DataPtr);
			return Main_DataPtr[Index];
		}
	}

public:
	FORCEINLINE void PrepareForWrite(const IVoxelDataOctreeMemory& Memory)
	{
		checkVoxelSlow(HasData());
		CheckState();
		if (bUseChannels)
		{
			// Allocate main
			Main_Allocate(Memory);
			checkVoxelSlow(Main_DataPtr);

			// Copy data over
			for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
			{
				Main_DataPtr[Index] = GetFromChannels(Index);
			}

			// Free channels
			for (auto& DataPtr : Channels_DataPtr)
			{
				if (DataPtr)
				{
					Channels_Deallocate(DataPtr, Memory);
				}
			}
			bUseChannels = false;
		}
		checkVoxelSlow(!bUseChannels);
		checkVoxelSlow(HasData());
		CheckState();
	}
	FORCEINLINE FVoxelMaterial& GetRef(int32 Index)
	{
		CheckBounds(Index);
		checkVoxelSlow(Main_DataPtr);
		return Main_DataPtr[Index];
	}
	FORCEINLINE void SetSingleValue(FVoxelMaterial SingleValue)
	{
		CheckState();
		checkVoxelSlow(!HasData());
		bUseChannels = true;
		for (int32 Channel = 0; Channel < NumChannels; Channel++)
		{
			Channels_SingleValue[Channel] = SingleValue.GetRaw(Channel);
		}
		CheckState();
	}

public:
	FORCEINLINE void CopyTo(FVoxelMaterial* RESTRICT DestPtr) const
	{
		checkVoxelSlow(DestPtr);
		checkVoxelSlow(HasData());
		if (bUseChannels)
		{
			for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
			{
				DestPtr[Index] = GetFromChannels(Index);
			}
		}
		else
		{
			checkVoxelSlow(Main_DataPtr);
			FMemory::Memcpy(DestPtr, Main_DataPtr, Main_MemorySize);
		}
	}
	
private:
	FORCEINLINE void CheckState() const
	{
		checkVoxelSlow(!(Main_DataPtr && bUseChannels));
		checkVoxelSlow(!bDirty || HasData());
	}
	FORCEINLINE static void CheckBounds(int32 Index)
	{
		checkVoxelSlow(0 <= Index && Index < VOXELS_PER_DATA_CHUNK);
	}

	FORCEINLINE FVoxelMaterial GetFromChannels(int32 Index) const
	{
		CheckBounds(Index);
		checkVoxelSlow(bUseChannels);

		FVoxelMaterial Material;
		for (int32 Channel = 0; Channel < NumChannels; Channel++)
		{
			if (const uint8* RESTRICT const DataPtr = Channels_DataPtr[Channel])
			{
				Material.GetRaw(Channel) = DataPtr[Index];
			}
			else
			{
				Material.GetRaw(Channel) = Channels_SingleValue[Channel];
			}
		}
		return Material;
	}
	
private:
	static constexpr int32 Main_MemorySize = VOXELS_PER_DATA_CHUNK * sizeof(FVoxelMaterial);
	static constexpr int32 Channels_MemorySize = VOXELS_PER_DATA_CHUNK * sizeof(uint8);
	
	void Main_Allocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();
		
		check(!Main_DataPtr);
		Main_DataPtr = static_cast<FVoxelMaterial*>(FMemory::Malloc(Main_MemorySize));

		TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Increase(Main_MemorySize, bDirty, Memory);
	}
	void Main_Deallocate(const IVoxelDataOctreeMemory& Memory)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(Main_DataPtr);
		FMemory::Free(Main_DataPtr);
		Main_DataPtr = nullptr;

		TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Decrease(Main_MemorySize, bDirty, Memory);
	}
	
	void Channels_Allocate(uint8* RESTRICT& DataPtr, const IVoxelDataOctreeMemory& Memory) const
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(!DataPtr);
		DataPtr = static_cast<uint8*>(FMemory::Malloc(Channels_MemorySize));

		TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Increase(Channels_MemorySize, bDirty, Memory);
	}
	void Channels_Deallocate(uint8* RESTRICT& DataPtr, const IVoxelDataOctreeMemory& Memory) const
	{
		VOXEL_SLOW_FUNCTION_COUNTER();

		check(DataPtr);
		FMemory::Free(DataPtr);
		DataPtr = nullptr;

		TVoxelDataOctreeLeafMemoryUsage<FVoxelMaterial>::Decrease(Channels_MemorySize, bDirty, Memory);
	}	
};