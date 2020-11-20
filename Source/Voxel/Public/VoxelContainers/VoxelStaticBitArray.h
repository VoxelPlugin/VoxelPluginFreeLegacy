// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"
#include "VoxelContainers/VoxelStaticArray.h"

template<uint32 InSize>
class TVoxelStaticBitArray
{
public:
	static constexpr uint32 NumBitsPerWord = 32;
	static constexpr uint32 Size = InSize;
	
	TVoxelStaticBitArray() = default;
	TVoxelStaticBitArray(EForceInit)
	{
		Clear();
	}

	void Clear()
	{
		Array.Memzero();
	}
	FORCEINLINE uint32* RESTRICT GetData()
	{
		return Array.GetData();
	}
	
	FORCEINLINE uint32 GetInternal(uint32 Index) const
	{
		return Array[Index];
	}
	FORCEINLINE uint32& GetInternal(uint32 Index)
	{
		return Array[Index];
	}

	FORCEINLINE void Set(uint32 Index, bool bValue)
	{
		checkVoxelSlow(Index < Size);

		const uint32 Mask = 1u << (Index % NumBitsPerWord);
		uint32& Value = Array[Index / NumBitsPerWord];

		if (bValue)
		{
			Value |= Mask;
		}
		else
		{
			Value &= ~Mask;
		}

		checkVoxelSlow(Test(Index) == bValue);
	}
	FORCEINLINE bool Test(uint32 Index) const
	{
		checkVoxelSlow(Index < Size);
		return Array[Index / NumBitsPerWord] & (1u << (Index % NumBitsPerWord));
	}
	FORCEINLINE bool TestAndClear(uint32 Index)
	{
		checkVoxelSlow(Index < Size);

		const uint32 Mask = 1u << (Index % NumBitsPerWord);
		uint32& Value = Array[Index / NumBitsPerWord];

		const bool bResult = Value & Mask;
		checkVoxelSlow(Test(Index) == bResult);
		Value &= ~Mask;
		checkVoxelSlow(!Test(Index));
		return bResult;
	}
	FORCEINLINE bool operator[](uint32 Index) const
	{
		return Test(Index);
	}
	
	FORCEINLINE void SetRange(uint32 Index, uint32 Num, bool bValue)
	{
		checkVoxelSlow(Index + Num <= Size);

		if (Num == 0)
		{
			return;
		}

		// Work out which uint32 index to set from, and how many
		const uint32 StartIndex = Index / NumBitsPerWord;
		uint32 Count = FVoxelUtilities::DivideCeil(Index + Num, NumBitsPerWord) - StartIndex;

		// Work out masks for the start/end of the sequence
		const uint32 StartMask = 0xFFFFFFFFu << (Index % NumBitsPerWord);
		const uint32 EndMask   = 0xFFFFFFFFu >> (NumBitsPerWord - (Index + Num) % NumBitsPerWord) % NumBitsPerWord;
		
		uint32* RESTRICT Data = Array.GetData() + StartIndex;
		if (bValue)
		{
			if (Count == 1)
			{
				*Data |= StartMask & EndMask;
			}
			else
			{
				*Data++ |= StartMask;
				Count -= 2;
				while (Count != 0)
				{
					*Data++ = ~0;
					--Count;
				}
				*Data |= EndMask;
			}
		}
		else
		{
			if (Count == 1)
			{
				*Data &= ~(StartMask & EndMask);
			}
			else
			{
				*Data++ &= ~StartMask;
				Count -= 2;
				while (Count != 0)
				{
					*Data++ = 0;
					--Count;
				}
				*Data &= ~EndMask;
			}
		}

#if VOXEL_DEBUG && 0
		for (uint32 It = Index; It < Index + Num; It++)
		{
			check(Test(It) == bValue);
		}
#endif
	}
	FORCEINLINE bool TestRange(uint32 Index, uint32 Num) const
	{
		const bool bResult = TestRangeImpl(Index, Num);

#if VOXEL_DEBUG && 0
		if (bResult)
		{
			for (uint32 It = Index; It < Index + Num; It++)
			{
				if (!ensure(Test(It)))
				{
					TestRangeImpl(Index, Num);
				}
			}
		}
		else
		{
			bool bValue = true;
			for (uint32 It = Index; It < Index + Num; It++)
			{
				bValue &= Test(It);
			}
			if (!ensure(!bValue))
			{
				TestRangeImpl(Index, Num);
			}
		}
#endif

		return bResult;
	}
	// Test a range, and clears it if all true
	FORCEINLINE bool TestAndClearRange(uint32 Index, uint32 Num)
	{
		checkVoxelSlow(Index + Num <= Size && Num > 0);

		// Work out which uint32 index to set from, and how many
		const uint32 StartIndex = Index / NumBitsPerWord;
		const uint32 Count = FVoxelUtilities::DivideCeil(Index + Num, NumBitsPerWord) - StartIndex;

		// Work out masks for the start/end of the sequence
		const uint32 StartMask = 0xFFFFFFFFu << (Index % NumBitsPerWord);
		const uint32 EndMask   = 0xFFFFFFFFu >> (NumBitsPerWord - (Index + Num) % NumBitsPerWord) % NumBitsPerWord;
		
		uint32* RESTRICT const Data = Array.GetData() + StartIndex;
		
		if (Count == 1)
		{
			const uint32 Mask = StartMask & EndMask;
			if ((*Data & Mask) != Mask)
			{
				return false;
			}
			
			*Data &= ~Mask;
			return true;
		}
		else
		{
			{
				uint32* RESTRICT LocalData = Data;
				uint32 LocalCount = Count;
				
				if ((*LocalData++ & StartMask) != StartMask)
				{
					return false;
				}

				LocalCount -= 2;
				while (LocalCount != 0)
				{
					if (*LocalData++ != 0xFFFFFFFFu)
					{
						return false;
					}
					--LocalCount;
				}

				if ((*LocalData & EndMask) != EndMask)
				{
					return false;
				}
			}
			
			{
				uint32* RESTRICT LocalData = Data;
				uint32 LocalCount = Count;
				
				*LocalData++ &= ~StartMask;
				LocalCount -= 2;
				while (LocalCount != 0)
				{
					*LocalData++ = 0;
					--LocalCount;
				}
				*LocalData &= ~EndMask;

				return true;
			}
		}
	}

private:
	TVoxelStaticArray<uint32, FVoxelUtilities::DivideCeil(Size, NumBitsPerWord)> Array;
	
	FORCEINLINE bool TestRangeImpl(uint32 Index, uint32 Num) const
	{
		checkVoxelSlow(Index + Num <= Size && Num > 0);

		// Work out which uint32 index to set from, and how many
		const uint32 StartIndex = Index / NumBitsPerWord;
		uint32 Count = FVoxelUtilities::DivideCeil(Index + Num, NumBitsPerWord) - StartIndex;

		// Work out masks for the start/end of the sequence
		const uint32 StartMask = 0xFFFFFFFFu << (Index % NumBitsPerWord);
		const uint32 EndMask   = 0xFFFFFFFFu >> (NumBitsPerWord - (Index + Num) % NumBitsPerWord) % NumBitsPerWord;
		
		const uint32* RESTRICT Data = Array.GetData() + StartIndex;
		
		if (Count == 1)
		{
			const uint32 Mask = StartMask & EndMask;
			if ((*Data & Mask) != Mask)
			{
				return false;
			}
		}
		else
		{
			if ((*Data++ & StartMask) != StartMask)
			{
				return false;
			}
			
			Count -= 2;
			while (Count != 0)
			{
				if ((*Data++ & ~0) != ~0)
				{
					return false;
				}
				--Count;
			}
			
			if ((*Data & EndMask) != EndMask)
			{
				return false;
			}
		}

		return true;
	}
};