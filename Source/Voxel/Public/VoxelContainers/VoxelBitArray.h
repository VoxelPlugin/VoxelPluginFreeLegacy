// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

// Copy of 4.24 TBitArray that supports 64 bit indices

/** Used to read/write a bit in the array as a bool. */
class FVoxelBitReference
{
public:

	FORCEINLINE FVoxelBitReference(uint32& InData,uint32 InMask)
	:	Data(InData)
	,	Mask(InMask)
	{}

	FORCEINLINE operator bool() const
	{
		 return (Data & Mask) != 0;
	}
	FORCEINLINE void operator=(const bool NewValue)
	{
		if(NewValue)
		{
			Data |= Mask;
		}
		else
		{
			Data &= ~Mask;
		}
	}
	FORCEINLINE void operator|=(const bool NewValue)
	{
		if (NewValue)
		{
			Data |= Mask;
		}
	}
	FORCEINLINE void operator&=(const bool NewValue)
	{
		if (!NewValue)
		{
			Data &= ~Mask;
		}
	}
	FORCEINLINE void AtomicSet(const bool NewValue)
	{
		if(NewValue)
		{
			if (!(Data & Mask))
			{
				while (1)
				{
					uint32 Current = Data;
					uint32 Desired = Current | Mask;
					if (Current == Desired || FPlatformAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
					{
						return;
					}
				}
			}
		}
		else
		{
			if (Data & Mask)
			{
				while (1)
				{
					uint32 Current = Data;
					uint32 Desired = Current & ~Mask;
					if (Current == Desired || FPlatformAtomics::InterlockedCompareExchange((volatile int32*)&Data, (int32)Desired, (int32)Current) == (int32)Current)
					{
						return;
					}
				}
			}
		}
	}
	FORCEINLINE FVoxelBitReference& operator=(const FVoxelBitReference& Copy)
	{
		// As this is emulating a reference, assignment should not rebind,
		// it should write to the referenced bit.
		*this = (bool)Copy;
		return *this;
	}

private:
	uint32& Data;
	uint32 Mask;
};


/** Used to read a bit in the array as a bool. */
class FVoxelConstBitReference
{
public:

	FORCEINLINE FVoxelConstBitReference(const uint32& InData,uint32 InMask)
	:	Data(InData)
	,	Mask(InMask)
	{}

	FORCEINLINE operator bool() const
	{
		 return (Data & Mask) != 0;
	}

private:
	const uint32& Data;
	uint32 Mask;
};


/** Used to reference a bit in an unspecified bit array. */
class FVoxelRelativeBitReference
{
public:
	FORCEINLINE explicit FVoxelRelativeBitReference(int64 BitIndex)
		: DWORDIndex(BitIndex >> /*VoxelNumBitsPerDWORDLogTwo*/int64(5))
		, Mask(1 << (BitIndex & (/*VoxelNumBitsPerDWORD*/int64(32) - 1)))
	{
	}

	int64  DWORDIndex;
	uint32 Mask;
};


/**
 * A dynamically sized bit array.
 * An array of Booleans.  They stored in one bit/Boolean.  There are iterators that efficiently iterate over only set bits.
 */
template<typename Allocator /*= FDefaultBitArrayAllocator*/>
class TVoxelBitArray
{
	friend class FScriptBitArray;

public:
	typedef typename Allocator::SizeType SizeType;

	static constexpr SizeType VoxelNumBitsPerDWORD = 32;
	static constexpr SizeType VoxelNumBitsPerDWORDLogTwo = 5;

	template<typename>
	friend class TConstSetBitIterator;

	template<typename,typename UE_26_ONLY(,bool)>
	friend class TConstDualSetBitIterator;

	template<typename>
	friend class TVoxelBitArray;

	/**
	 * Minimal initialization constructor.
	 * @param Value - The value to initial the bits to.
	 * @param InNumBits - The initial number of bits in the array.
	 */
	explicit TVoxelBitArray( const bool Value = false, const SizeType InNumBits = 0 )
	:	NumBits(0)
	,	MaxBits(0)
	{
		Init(Value,InNumBits);
	}

	/**
	 * Move constructor.
	 */
	FORCEINLINE TVoxelBitArray(TVoxelBitArray&& Other)
	{
		MoveOrCopy(*this, Other);
	}

	/**
	 * Copy constructor.
	 */
	FORCEINLINE TVoxelBitArray(const TVoxelBitArray& Copy)
	:	NumBits(0)
	,	MaxBits(0)
	{
		*this = Copy;
	}

	/**
	 * Move assignment.
	 */
	FORCEINLINE TVoxelBitArray& operator=(TVoxelBitArray&& Other)
	{
		if (this != &Other)
		{
			MoveOrCopy(*this, Other);
		}

		return *this;
	}

	/**
	 * Assignment operator.
	 */
	FORCEINLINE TVoxelBitArray& operator=(const TVoxelBitArray& Copy)
	{
		// check for self assignment since we don't use swap() mechanic
		if( this == &Copy )
		{
			return *this;
		}

		Empty(Copy.Num());
		NumBits = Copy.NumBits;
		if(NumBits)
		{
			const SizeType NumDWORDs = FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD);
			FMemory::Memcpy(GetWordData(),Copy.GetWordData(),NumDWORDs * sizeof(uint32));
		}
		return *this;
	}

	template<typename OtherAllocator>
	FORCEINLINE TVoxelBitArray& operator=(const TVoxelBitArray<OtherAllocator>& Copy)
	{
		Empty(Copy.Num());
		NumBits = Copy.NumBits;
		if(NumBits)
		{
			const SizeType NumDWORDs = FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD);
			FMemory::Memcpy(GetWordData(),Copy.GetWordData(),NumDWORDs * sizeof(uint32));
		}
		return *this;
	}

	FORCEINLINE bool operator==(const TVoxelBitArray<Allocator>& Other) const
	{
		if (Num() != Other.Num())
		{
			return false;
		}

		int NumBytes = FMath::DivideAndRoundUp(NumBits, VoxelNumBitsPerDWORD) * sizeof(uint32);
		return FMemory::Memcmp(GetWordData(), Other.GetWordData(), NumBytes) == 0;
	}

	FORCEINLINE bool operator<(const TVoxelBitArray<Allocator>& Other) const
	{
		//sort by length
		if (Num() != Other.Num())
		{
			return Num() < Other.Num();
		}

		uint32 NumWords = FMath::DivideAndRoundUp(Num(), VoxelNumBitsPerDWORD);
		const uint32* Data0 = GetWordData();
		const uint32* Data1 = Other.GetWordData();

		//lexicographically compare
		for (uint32 i = 0; i < NumWords; i++)
		{
			if (Data0[i] != Data1[i])
			{
				return Data0[i] < Data1[i];
			}
		}
		return false;
	}

	FORCEINLINE bool operator!=(const TVoxelBitArray<Allocator>& Other)
	{
		return !(*this == Other);
	}

private:
	template <typename BitArrayType>
	static FORCEINLINE typename TEnableIf<TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
	{
		ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

		ToArray  .NumBits = FromArray.NumBits;
		ToArray  .MaxBits = FromArray.MaxBits;
		FromArray.NumBits = 0;
		FromArray.MaxBits = 0;
	}

	template <typename BitArrayType>
	static FORCEINLINE typename TEnableIf<!TContainerTraits<BitArrayType>::MoveWillEmptyContainer>::Type MoveOrCopy(BitArrayType& ToArray, BitArrayType& FromArray)
	{
		ToArray = FromArray;
	}

public:
	/**
	 * Serializer
	 */
	friend FArchive& operator<<(FArchive& Ar, TVoxelBitArray& BitArray)
	{
		// serialize number of bits
		Ar << BitArray.NumBits;

		if (Ar.IsLoading())
		{
			// no need for slop when reading
			BitArray.MaxBits = BitArray.NumBits;

			// allocate room for new bits
			BitArray.Realloc(0);
		}

		// calc the number of dwords for all the bits
		const SizeType NumDWORDs = FMath::DivideAndRoundUp(BitArray.NumBits, VoxelNumBitsPerDWORD);

		// serialize the data as one big chunk
		Ar.Serialize(BitArray.GetWordData(), NumDWORDs * sizeof(uint32));

		return Ar;
	}

	/**
	 * Adds a bit to the array with the given value.
	 * @return The index of the added bit.
	 */
	SizeType Add(const bool Value)
	{
		const SizeType Index = NumBits;

		Reserve(Index + 1);
		++NumBits;
		(*this)[Index] = Value;

		return Index;
	}

	/**
	 * Adds multiple bits to the array with the given value.
	 * @return The index of the first added bit.
	 */
	SizeType Add(const bool Value, SizeType NumToAdd)
	{
		const SizeType Index = NumBits;

		if (NumToAdd > 0)
		{
			Reserve(Index + NumToAdd);
			NumBits += NumToAdd;
			for (SizeType It = Index, End = It + NumToAdd; It != End; ++It)
			{
				(*this)[It] = Value;
			}
		}

		return Index;
	}
	
	void SetNumUninitialized(SizeType NewNum)
	{
		Reserve(NewNum);
		NumBits = NewNum;
		checkVoxelSlow(NumBits <= MaxBits);
	}
	SizeType AddUninitialized(SizeType NumToAdd)
	{
		const SizeType Index = NumBits;

		if (NumToAdd > 0)
		{
			Reserve(Index + NumToAdd);
			NumBits += NumToAdd;
		}

		return Index;
	}


	/**
	 * Removes all bits from the array, potentially leaving space allocated for an expected number of bits about to be added.
	 * @param ExpectedNumBits - The expected number of bits about to be added.
	 */
	void Empty(SizeType ExpectedNumBits = 0)
	{
		NumBits = 0;

		ExpectedNumBits = FMath::DivideAndRoundUp(ExpectedNumBits, VoxelNumBitsPerDWORD) * VoxelNumBitsPerDWORD;
		// If the expected number of bits doesn't match the allocated number of bits, reallocate.
		if(MaxBits != ExpectedNumBits)
		{
			MaxBits = ExpectedNumBits;
			Realloc(0);
		}
	}

	/**
	 * Reserves memory such that the array can contain at least Number bits.
	 *
	 * @Number  The number of bits to reserve space for.
	 */
	void Reserve(SizeType Number)
	{
		if (Number > MaxBits)
		{
			const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackGrow(
				FMath::DivideAndRoundUp(Number,  VoxelNumBitsPerDWORD),
				FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD),
				sizeof(uint32)
				);
			MaxBits = MaxDWORDs * VoxelNumBitsPerDWORD;
			Realloc(NumBits);
		}
	}

	/**
	 * Removes all bits from the array retaining any space already allocated.
	 */
	void Reset()
	{
		// We need this because iterators often use whole DWORDs when masking, which includes off-the-end elements
		FMemory::Memset(GetWordData(), 0, FMath::DivideAndRoundUp(NumBits, VoxelNumBitsPerDWORD) * sizeof(uint32));

		NumBits = 0;
	}

	/**
	 * Resets the array's contents.
	 * @param Value - The value to initial the bits to.
	 * @param NumBits - The number of bits in the array.
	 */
	void Init(bool Value,SizeType InNumBits)
	{
		Empty(InNumBits);
		if(InNumBits)
		{
			NumBits = InNumBits;
			FMemory::Memset(GetWordData(),Value ? 0xff : 0, FMath::DivideAndRoundUp(NumBits, VoxelNumBitsPerDWORD) * sizeof(uint32));
		}
	}

	/**
	 * Sets or unsets a range of bits within the array.
	 * @param  Index  The index of the first bit to set.
	 * @param  Num    The number of bits to set.
	 * @param  Value  The value to set the bits to.
	 */
	FORCENOINLINE void SetRange(SizeType Index, SizeType Num, bool Value)
	{
		checkVoxelSlow(Index >= 0 && Num >= 0 && Index + Num <= NumBits);

		if (Num == 0)
		{
			return;
		}

		// Work out which uint32 index to set from, and how many
		uint32 StartIndex = Index / VoxelNumBitsPerDWORD;
		uint32 Count      = (Index + Num + (VoxelNumBitsPerDWORD - 1)) / VoxelNumBitsPerDWORD - StartIndex;

		// Work out masks for the start/end of the sequence
		uint32 StartMask  = 0xFFFFFFFFu << (Index % VoxelNumBitsPerDWORD);
		uint32 EndMask    = 0xFFFFFFFFu >> (VoxelNumBitsPerDWORD - (Index + Num) % VoxelNumBitsPerDWORD) % VoxelNumBitsPerDWORD;

		uint32* Data = GetWordData() + StartIndex;
		if (Value)
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
	}

	/**
	 * Removes bits from the array.
	 * @param BaseIndex - The index of the first bit to remove.
	 * @param NumBitsToRemove - The number of consecutive bits to remove.
	 */
	void RemoveAt(SizeType BaseIndex,SizeType NumBitsToRemove = 1)
	{
		checkVoxelSlow(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= NumBits);

		if (BaseIndex + NumBitsToRemove != NumBits)
		{
			// Until otherwise necessary, this is an obviously correct implementation rather than an efficient implementation.
			FIterator WriteIt(*this);
			for(FConstIterator ReadIt(*this);ReadIt;++ReadIt)
			{
				// If this bit isn't being removed, write it back to the array at its potentially new index.
				if(ReadIt.GetIndex() < BaseIndex || ReadIt.GetIndex() >= BaseIndex + NumBitsToRemove)
				{
					if(WriteIt.GetIndex() != ReadIt.GetIndex())
					{
						WriteIt.GetValue() = (bool)ReadIt.GetValue();
					}
					++WriteIt;
				}
			}
		}
		NumBits -= NumBitsToRemove;
	}

	/* Removes bits from the array by swapping them with bits at the end of the array.
	 * This is mainly implemented so that other code using TArray::RemoveSwap will have
	 * matching indices.
 	 * @param BaseIndex - The index of the first bit to remove.
	 * @param NumBitsToRemove - The number of consecutive bits to remove.
	 */
	void RemoveAtSwap( SizeType BaseIndex, SizeType NumBitsToRemove=1 )
	{
		checkVoxelSlow(BaseIndex >= 0 && NumBitsToRemove >= 0 && BaseIndex + NumBitsToRemove <= NumBits);
		if( BaseIndex < NumBits - NumBitsToRemove )
		{
			// Copy bits from the end to the region we are removing
			for( SizeType Index=0;Index<NumBitsToRemove;Index++ )
			{
#if PLATFORM_MAC || PLATFORM_LINUX
				// Clang compiler doesn't understand the short syntax, so let's be explicit
				SizeType FromIndex = NumBits - NumBitsToRemove + Index;
				FVoxelConstBitReference From(GetWordData()[FromIndex / VoxelNumBitsPerDWORD],1 << (FromIndex & (VoxelNumBitsPerDWORD - 1)));

				SizeType ToIndex = BaseIndex + Index;
				FVoxelBitReference To(GetWordData()[ToIndex / VoxelNumBitsPerDWORD],1 << (ToIndex & (VoxelNumBitsPerDWORD - 1)));

				To = (bool)From;
#else
				(*this)[BaseIndex + Index] = (bool)(*this)[NumBits - NumBitsToRemove + Index];
#endif
			}
		}

		// Remove the bits from the end of the array.
		NumBits -= NumBitsToRemove;
	}
	

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 * @return number of bytes allocated by this container
	 */
	uint32 GetAllocatedSize( void ) const
	{
		return FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD) * sizeof(uint32);
	}

	/** Tracks the container's memory use through an archive. */
	void CountBytes(FArchive& Ar) const
	{
		Ar.CountBytes(
			FMath::DivideAndRoundUp(NumBits, VoxelNumBitsPerDWORD) * sizeof(uint32),
			FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD) * sizeof(uint32)
		);
	}

	/**
	 * Finds the first true/false bit in the array, and returns the bit index.
	 * If there is none, INDEX_NONE is returned.
	 */
	SizeType Find(bool bValue) const
	{
		// Iterate over the array until we see a word with a matching bit
		const uint32 Test = bValue ? 0u : (uint32)-1;

		const uint32* RESTRICT DwordArray = GetWordData();
		const SizeType LocalNumBits = NumBits;
		const SizeType DwordCount = FMath::DivideAndRoundUp(LocalNumBits, VoxelNumBitsPerDWORD);
		SizeType DwordIndex = 0;
		while (DwordIndex < DwordCount && DwordArray[DwordIndex] == Test)
		{
			++DwordIndex;
		}

		if (DwordIndex < DwordCount)
		{
			// If we're looking for a false, then we flip the bits - then we only need to find the first one bit
			const uint32 Bits = bValue ? (DwordArray[DwordIndex]) : ~(DwordArray[DwordIndex]);
			ASSUME(Bits != 0);
			const SizeType LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << VoxelNumBitsPerDWORDLogTwo);
			if (LowestBitIndex < LocalNumBits)
			{
				return LowestBitIndex;
			}
		}

		return INDEX_NONE;
	}

	/**
	* Finds the last true/false bit in the array, and returns the bit index.
	* If there is none, INDEX_NONE is returned.
	*/
	SizeType FindLast(bool bValue) const 
	{
		const SizeType LocalNumBits = NumBits;

		// Get the correct mask for the last word
		uint32 SlackIndex = ((LocalNumBits - 1) % VoxelNumBitsPerDWORD) + 1;
		uint32 Mask = ~0u >> (VoxelNumBitsPerDWORD - SlackIndex);

		// Iterate over the array until we see a word with a zero bit.
		uint32 DwordIndex = FMath::DivideAndRoundUp(LocalNumBits, VoxelNumBitsPerDWORD);
		const uint32* RESTRICT DwordArray = GetWordData();
		const uint32 Test = bValue ? 0u : ~0u;
		for (;;)
		{
			if (DwordIndex == 0)
			{
				return INDEX_NONE;
			}
			--DwordIndex;
			if ((DwordArray[DwordIndex] & Mask) != (Test & Mask))
			{
				break;
			}
			Mask = ~0u;
		}

		// Flip the bits, then we only need to find the first one bit -- easy.
		const uint32 Bits = (bValue ? DwordArray[DwordIndex] : ~DwordArray[DwordIndex]) & Mask;
		ASSUME(Bits != 0);

		uint32 BitIndex = (VoxelNumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);

		SizeType Result = BitIndex + (DwordIndex << VoxelNumBitsPerDWORDLogTwo);
		return Result;
	}

	FORCEINLINE bool Contains(bool bValue) const
	{
		return Find(bValue) != INDEX_NONE;
	}

	/**
	 * Finds the first zero bit in the array, sets it to true, and returns the bit index.
	 * If there is none, INDEX_NONE is returned.
	 */
	SizeType FindAndSetFirstZeroBit(SizeType ConservativeStartIndex = 0)
	{
		// Iterate over the array until we see a word with a zero bit.
		uint32* RESTRICT DwordArray = GetWordData();
		const SizeType LocalNumBits = NumBits;
		const SizeType DwordCount = FMath::DivideAndRoundUp(LocalNumBits, VoxelNumBitsPerDWORD);
		SizeType DwordIndex = FMath::DivideAndRoundDown(ConservativeStartIndex, VoxelNumBitsPerDWORD);
		while (DwordIndex < DwordCount && DwordArray[DwordIndex] == (uint32)-1)
		{
			++DwordIndex;
		}

		if (DwordIndex < DwordCount)
		{
			// Flip the bits, then we only need to find the first one bit -- easy.
			const uint32 Bits = ~(DwordArray[DwordIndex]);
			ASSUME(Bits != 0);
			const uint32 LowestBit = (Bits) & (-(int32)Bits);
			const SizeType LowestBitIndex = FMath::CountTrailingZeros(Bits) + (DwordIndex << VoxelNumBitsPerDWORDLogTwo);
			if (LowestBitIndex < LocalNumBits)
			{
				DwordArray[DwordIndex] |= LowestBit;
				return LowestBitIndex;
			}
		}

		return INDEX_NONE;
	}

	/**
	 * Finds the last zero bit in the array, sets it to true, and returns the bit index.
	 * If there is none, INDEX_NONE is returned.
	 */
	SizeType FindAndSetLastZeroBit()
	{
		const SizeType LocalNumBits = NumBits;

		// Get the correct mask for the last word
		uint32 SlackIndex = ((LocalNumBits - 1) % VoxelNumBitsPerDWORD) + 1;
		uint32 Mask = ~0u >> (VoxelNumBitsPerDWORD - SlackIndex);

		// Iterate over the array until we see a word with a zero bit.
		uint32 DwordIndex = FMath::DivideAndRoundUp(LocalNumBits, VoxelNumBitsPerDWORD);
		uint32* RESTRICT DwordArray = GetWordData();
		for (;;)
		{
			if (DwordIndex == 0)
			{
				return INDEX_NONE;
			}
			--DwordIndex;
			if ((DwordArray[DwordIndex] & Mask) != Mask)
			{
				break;
			}
			Mask = ~0u;
		}

		// Flip the bits, then we only need to find the first one bit -- easy.
		const uint32 Bits = ~DwordArray[DwordIndex] & Mask;
		ASSUME(Bits != 0);

		uint32 BitIndex = (VoxelNumBitsPerDWORD - 1) - FMath::CountLeadingZeros(Bits);
		DwordArray[DwordIndex] |= 1u << BitIndex;

		SizeType Result = BitIndex + (DwordIndex << VoxelNumBitsPerDWORDLogTwo);
		return Result;
	}

	// Accessors.
	FORCEINLINE bool IsValidIndex(SizeType InIndex) const
	{
		return InIndex >= 0 && InIndex < NumBits;
	}

	static double GetTypeSize()
	{
		return 1. / VoxelNumBitsPerDWORD;
	}

	FORCEINLINE SizeType Num() const { return NumBits; }
	FORCEINLINE FVoxelBitReference operator[](SizeType Index)
	{
		checkVoxelSlow(Index>=0 && Index<NumBits);
		return FVoxelBitReference(
			GetWordData()[Index / VoxelNumBitsPerDWORD],
			1 << (Index & (VoxelNumBitsPerDWORD - 1))
			);
	}
	FORCEINLINE FVoxelConstBitReference operator[](SizeType Index) const
	{
		checkVoxelSlow(Index>=0 && Index<NumBits);
		return FVoxelConstBitReference(
			GetWordData()[Index / VoxelNumBitsPerDWORD],
			1 << (Index & (VoxelNumBitsPerDWORD - 1))
			);
	}
	FORCEINLINE FVoxelBitReference AccessCorrespondingBit(const FVoxelRelativeBitReference& RelativeReference)
	{
		checkVoxelSlow(RelativeReference.Mask);
		checkVoxelSlow(RelativeReference.DWORDIndex >= 0);
		checkVoxelSlow(((uint32)RelativeReference.DWORDIndex + 1) * VoxelNumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
		return FVoxelBitReference(
			GetWordData()[RelativeReference.DWORDIndex],
			RelativeReference.Mask
			);
	}
	FORCEINLINE const FVoxelConstBitReference AccessCorrespondingBit(const FVoxelRelativeBitReference& RelativeReference) const
	{
		checkVoxelSlow(RelativeReference.Mask);
		checkVoxelSlow(RelativeReference.DWORDIndex >= 0);
		checkVoxelSlow(((uint32)RelativeReference.DWORDIndex + 1) * VoxelNumBitsPerDWORD - 1 - FMath::CountLeadingZeros(RelativeReference.Mask) < (uint32)NumBits);
		return FVoxelConstBitReference(
			GetWordData()[RelativeReference.DWORDIndex],
			RelativeReference.Mask
			);
	}

	/** BitArray iterator. */
	class FIterator : public FVoxelRelativeBitReference
	{
	public:
		FORCEINLINE FIterator(TVoxelBitArray<Allocator>& InArray,SizeType StartIndex = 0)
		:	FVoxelRelativeBitReference(StartIndex)
		,	Array(InArray)
		,	Index(StartIndex)
		{
		}
		FORCEINLINE FIterator& operator++()
		{
			++Index;
			this->Mask <<= 1;
			if(!this->Mask)
			{
				// Advance to the next uint32.
				this->Mask = 1;
				++this->DWORDIndex;
			}
			return *this;
		}
		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{ 
			return Index < Array.Num(); 
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const 
		{
			return !(bool)*this;
		}

		FORCEINLINE FVoxelBitReference GetValue() const { return FVoxelBitReference(Array.GetWordData()[this->DWORDIndex],this->Mask); }
		FORCEINLINE SizeType GetIndex() const { return Index; }
	
	private:
		TVoxelBitArray<Allocator>& Array;
		SizeType Index;
	};

	/** Const BitArray iterator. */
	class FConstIterator : public FVoxelRelativeBitReference
	{
	public:
		FORCEINLINE FConstIterator(const TVoxelBitArray<Allocator>& InArray,SizeType StartIndex = 0)
		:	FVoxelRelativeBitReference(StartIndex)
		,	Array(InArray)
		,	Index(StartIndex)
		{
		}
		FORCEINLINE FConstIterator& operator++()
		{
			++Index;
			this->Mask <<= 1;
			if(!this->Mask)
			{
				// Advance to the next uint32.
				this->Mask = 1;
				++this->DWORDIndex;
			}
			return *this;
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{ 
			return Index < Array.Num(); 
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const 
		{
			return !(bool)*this;
		}

		FORCEINLINE FVoxelConstBitReference GetValue() const { return FVoxelConstBitReference(Array.GetWordData()[this->DWORDIndex],this->Mask); }
		FORCEINLINE SizeType GetIndex() const { return Index; }
	
	private:
		const TVoxelBitArray<Allocator>& Array;
		SizeType Index;
	};

	/** Const reverse iterator. */
	class FConstReverseIterator : public FVoxelRelativeBitReference
	{
	public:
		FORCEINLINE FConstReverseIterator(const TVoxelBitArray<Allocator>& InArray)
			:	FVoxelRelativeBitReference(InArray.Num() - 1)
			,	Array(InArray)
			,	Index(InArray.Num() - 1)
		{
		}
		FORCEINLINE FConstReverseIterator& operator++()
		{
			--Index;
			this->Mask >>= 1;
			if(!this->Mask)
			{
				// Advance to the next uint32.
				this->Mask = (1 << (VoxelNumBitsPerDWORD-1));
				--this->DWORDIndex;
			}
			return *this;
		}

		/** conversion to "bool" returning true if the iterator is valid. */
		FORCEINLINE explicit operator bool() const
		{ 
			return Index >= 0; 
		}
		/** inverse of the "bool" operator */
		FORCEINLINE bool operator !() const 
		{
			return !(bool)*this;
		}

		FORCEINLINE FVoxelConstBitReference GetValue() const { return FVoxelConstBitReference(Array.GetWordData()[this->DWORDIndex],this->Mask); }
		FORCEINLINE SizeType GetIndex() const { return Index; }
	private:
		const TVoxelBitArray<Allocator>& Array;
		SizeType Index;
	};

	FORCEINLINE const uint32* GetWordData() const
	{
		return (uint32*)AllocatorInstance.GetAllocation();
	}

	FORCEINLINE uint32* GetWordData()
	{
		return (uint32*)AllocatorInstance.GetAllocation();
	}

	template<typename T>
	operator TArray<T> () const
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		TArray<T> Result;
		Result.Reserve(NumBits);
		for (SizeType Index = 0; Index < NumBits; Index++)
		{
			Result.Add((*this)[Index]);
		}
		return Result;
	}

private:
	typedef typename Allocator::template ForElementType<uint32> AllocatorType;

	AllocatorType AllocatorInstance;
	SizeType         NumBits;
	SizeType         MaxBits;

	FORCENOINLINE void Realloc(SizeType PreviousNumBits)
	{
		const SizeType PreviousNumDWORDs = FMath::DivideAndRoundUp(PreviousNumBits, VoxelNumBitsPerDWORD);
		const SizeType MaxDWORDs = FMath::DivideAndRoundUp(MaxBits, VoxelNumBitsPerDWORD);

		AllocatorInstance.ResizeAllocation(PreviousNumDWORDs,MaxDWORDs,sizeof(uint32));

		if(MaxDWORDs)
		{
			// Reset the newly allocated slack DWORDs.
			FMemory::Memzero((uint32*)AllocatorInstance.GetAllocation() + PreviousNumDWORDs,(MaxDWORDs - PreviousNumDWORDs) * sizeof(uint32));
		}
	}
};

template<typename Allocator>
struct TContainerTraits<TVoxelBitArray<Allocator> > : public TContainerTraitsBase<TVoxelBitArray<Allocator> >
{
	static_assert(TAllocatorTraits<Allocator>::SupportsMove, "TVoxelBitArray no longer supports move-unaware allocators");
	enum { MoveWillEmptyContainer = TAllocatorTraits<Allocator>::SupportsMove };
};

template<typename Allocator>
auto& GetData(TVoxelBitArray<Allocator>& Array) { return Array; }
template<typename Allocator>
auto& GetData(const TVoxelBitArray<Allocator>& Array) { return Array; }

template<typename Allocator>
auto GetNum(const TVoxelBitArray<Allocator>& Array) { return Array.Num(); }

using FVoxelBitArray32 = TVoxelBitArray<FDefaultBitArrayAllocator>;
using FVoxelBitArray64 = TVoxelBitArray<FDefaultAllocator64>;