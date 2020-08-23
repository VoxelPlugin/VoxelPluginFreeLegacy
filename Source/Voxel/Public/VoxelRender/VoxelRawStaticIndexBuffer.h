// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "RawIndexBuffer.h"
#include "VoxelMinimal.h"

/**
 * Constructor isn't exported in the engine definition
 */
class VOXEL_API FVoxelRawStaticIndexBuffer : public FIndexBuffer
{
public:	
	/**
	 * Initialization constructor.
	 * @param InNeedsCPUAccess	True if resource array data should be accessible by the CPU.
	 */
	FVoxelRawStaticIndexBuffer(bool InNeedsCPUAccess);

	/**
	 * Sets a single index value.  Consider using SetIndices() instead if you're setting a lot of indices.
	 * @param	At	The index of the index to set
	 * @param	NewIndexValue	The index value
	 */
	inline void SetIndex( const uint32 At, const uint32 NewIndexValue )
	{
		check( At >= 0 && At < uint32(IndexStorage.Num()) );

		if( b32Bit )
		{
			uint32* Indices32Bit = reinterpret_cast<uint32*>(IndexStorage.GetData());
			Indices32Bit[ At ] = NewIndexValue;
		}
		else
		{
			uint16* Indices16Bit = reinterpret_cast<uint16*>(IndexStorage.GetData());
			Indices16Bit[ At ] = uint16(NewIndexValue);
		}
	}

	/**
	 * Set the indices stored within this buffer.
	 * @param InIndices		The new indices to copy in to the buffer.
	 * @param DesiredStride	The desired stride (16 or 32 bits).
	 */
	void SetIndices(const TArray<uint32>& InIndices, EIndexBufferStride::Type DesiredStride);

	// Stride set from the num
	void AllocateData(int32 NumInIndices);
	
	/**
	 * Insert indices at the given position in the buffer
	 * @param	At					Index to insert at
	 * @param	IndicesToAppend		Pointer to the array of indices to insert
	 * @param	NumIndicesToAppend	How many indices are in the IndicesToAppend array
	 */
	void InsertIndices(uint32 At, const uint32* IndicesToAppend, uint32 NumIndicesToAppend );

	/**
	 * Append indices to the end of the buffer
	 * @param	IndicesToAppend		Pointer to the array of indices to add to the end
	 * @param	NumIndicesToAppend	How many indices are in the IndicesToAppend array
	 */
	void AppendIndices( const uint32* IndicesToAppend, uint32 NumIndicesToAppend );

	/** @return Gets a specific index value */
	inline uint32 GetIndex(const uint32 At) const
	{
		checkVoxelSlow(At >= 0 && At < uint32(IndexStorage.Num()));
		uint32 IndexValue;
		if (b32Bit)
		{
			const uint32* SrcIndices32Bit = reinterpret_cast<const uint32*>(IndexStorage.GetData());
			IndexValue = SrcIndices32Bit[At];
		}
		else
		{
			const uint16* SrcIndices16Bit = reinterpret_cast<const uint16*>(IndexStorage.GetData());
			IndexValue = SrcIndices16Bit[At];
		}

		return IndexValue;
	}

	inline const uint32* RESTRICT GetData_32() const
	{
		check(b32Bit);
		return reinterpret_cast<const uint32*>(IndexStorage.GetData());
	}
	inline const uint16* RESTRICT GetData_16() const
	{
		check(!b32Bit);
		return reinterpret_cast<const uint16*>(IndexStorage.GetData());
	}

	/**
	 * Removes indices from the buffer
	 *
	 * @param	At	The index of the first index to remove
	 * @param	NumIndicesToRemove	How many indices to remove
	 */
	void RemoveIndicesAt(uint32 At, uint32 NumIndicesToRemove );

	/**
	 * Retrieve a copy of the indices in this buffer. Only valid if created with
	 * NeedsCPUAccess set to true or the resource has not yet been initialized.
	 * @param OutIndices	Array in which to store the copy of the indices.
	 */
	void GetCopy(TArray<uint32>& OutIndices) const;

	/**
	 * Get the direct read access to index data 
	 * Only valid if NeedsCPUAccess = true and indices are 16 bit
	 */
	const uint16* AccessStream16() const;

	/**
	 * Retrieves an array view in to the index buffer. The array view allows code
	 * to retrieve indices as 32-bit regardless of how they are stored internally
	 * without a copy. The array view is valid only if:
	 *		The buffer was created with NeedsCPUAccess = true
	 *		  OR the resource has not yet been initialized
	 *		  AND SetIndices has not been called since.
	 */
	FIndexArrayView GetArrayView() const;

	/**
	 * Computes the number of indices stored in this buffer.
	 */
	FORCEINLINE int32 GetNumIndices() const
	{
		return NumIndices;
	}

	/**
	 * Computes the amount of memory allocated to store the indices.
	 */
	FORCEINLINE uint32 GetAllocatedSize() const
	{
		return IndexStorage.GetAllocatedSize();
	}

	/**
	 * Serialization.
	 * @param	Ar				Archive to serialize with
	 * @param	bNeedsCPUAccess	Whether the elements need to be accessed by the CPU
	 */
	void Serialize(FArchive& Ar, bool bNeedsCPUAccess);

    /**
     * Discard
     * discards the serialized data when it is not needed
     */
    void Discard();
    
	// FRenderResource interface.
	virtual void InitRHI() override;

	inline bool Is32Bit() const { return b32Bit; }

private:
	/** Storage for indices. */
	TResourceArray<uint8, INDEXBUFFER_ALIGNMENT> IndexStorage;
	/** 32bit or 16bit? */
	bool b32Bit;
	/** The cached number of indices. */
	uint32 NumIndices = 0;

	void UpdateCachedNumIndices()
	{
		NumIndices = b32Bit ? (IndexStorage.Num() / 4) : (IndexStorage.Num() / 2);
	}
};