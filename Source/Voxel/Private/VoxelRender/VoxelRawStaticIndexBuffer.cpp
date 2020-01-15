// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelRawStaticIndexBuffer.h"

FVoxelRawStaticIndexBuffer::FVoxelRawStaticIndexBuffer(bool InNeedsCPUAccess)
	: IndexStorage(InNeedsCPUAccess)
	, b32Bit(false)
{
}

void FVoxelRawStaticIndexBuffer::SetIndices(const TArray<uint32>& InIndices, EIndexBufferStride::Type DesiredStride)
{
	const int32 NumInIndices = InIndices.Num();
	bool bShouldUse32Bit = false;

	// Figure out if we should store the indices as 16 or 32 bit.
	if (DesiredStride == EIndexBufferStride::Force32Bit)
	{
		bShouldUse32Bit = true;
	}
	else if (DesiredStride == EIndexBufferStride::AutoDetect)
	{
		int32 i = 0;
		while (!bShouldUse32Bit && i < NumInIndices)
		{
			bShouldUse32Bit = InIndices[i] > MAX_uint16;
			i++;
		}
	}

	// Allocate storage for the indices.
	const int32 IndexStride = bShouldUse32Bit ? sizeof(uint32) : sizeof(uint16);
	IndexStorage.Empty(IndexStride * NumInIndices);
	IndexStorage.AddUninitialized(IndexStride * NumInIndices);

	// Store them!
	if (bShouldUse32Bit)
	{
		// If the indices are 32 bit we can just do a memcpy.
		check(IndexStorage.Num() == InIndices.Num() * InIndices.GetTypeSize());
		FMemory::Memcpy(IndexStorage.GetData(),InIndices.GetData(),IndexStorage.Num());
		b32Bit = true;
	}
	else
	{
		// Copy element by element demoting 32-bit integers to 16-bit.
		check(IndexStorage.Num() == InIndices.Num() * sizeof(uint16));
		uint16* DestIndices16Bit = reinterpret_cast<uint16*>(IndexStorage.GetData());
		for (int32 i = 0; i < NumInIndices; ++i)
		{
			DestIndices16Bit[i] = InIndices[i];
		}
		b32Bit = false;
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::AllocateData(int32 NumInIndices)
{
	const bool bShouldUse32Bit = NumInIndices > MAX_uint16;

	// Allocate storage for the indices.
	const int32 IndexStride = bShouldUse32Bit ? sizeof(uint32) : sizeof(uint16);
	IndexStorage.Empty(IndexStride * NumInIndices);
	IndexStorage.AddUninitialized(IndexStride * NumInIndices);

	b32Bit = bShouldUse32Bit;

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::InsertIndices( const uint32 At, const uint32* IndicesToAppend, const uint32 NumIndicesToAppend )
{
	if( NumIndicesToAppend > 0 )
	{
		const uint32 IndexStride = b32Bit ? sizeof( uint32 ) : sizeof( uint16 );

		IndexStorage.InsertUninitialized( At * IndexStride, NumIndicesToAppend * IndexStride );
		uint8* const DestIndices = &IndexStorage[ At * IndexStride ];

		if( IndicesToAppend )
		{
			if( b32Bit )
			{
				// If the indices are 32 bit we can just do a memcpy.
				FMemory::Memcpy( DestIndices, IndicesToAppend, NumIndicesToAppend * IndexStride );
			}
			else
			{
				// Copy element by element demoting 32-bit integers to 16-bit.
				uint16* DestIndices16Bit = reinterpret_cast<uint16*>(DestIndices);
				for( uint32 Index = 0; Index < NumIndicesToAppend; ++Index )
				{
					DestIndices16Bit[ Index ] = IndicesToAppend[ Index ];
				}
			}
		}
		else
		{
			// If no indices to insert were supplied, just clear the buffer
			FMemory::Memset( DestIndices, 0, NumIndicesToAppend * IndexStride );
		}
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::AppendIndices( const uint32* IndicesToAppend, const uint32 NumIndicesToAppend )
{
	InsertIndices( b32Bit ? IndexStorage.Num() / 4 : IndexStorage.Num() / 2, IndicesToAppend, NumIndicesToAppend );
}

void FVoxelRawStaticIndexBuffer::RemoveIndicesAt( const uint32 At, const uint32 NumIndicesToRemove )
{
	if( NumIndicesToRemove > 0 )
	{
		const int32 IndexStride = b32Bit ? sizeof( uint32 ) : sizeof( uint16 );
		IndexStorage.RemoveAt( At * IndexStride, NumIndicesToRemove * IndexStride );
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::GetCopy(TArray<uint32>& OutIndices) const
{
	OutIndices.Empty(NumIndices);
	OutIndices.AddUninitialized(NumIndices);

	if (b32Bit)
	{
		// If the indices are 32 bit we can just do a memcpy.
		check(IndexStorage.Num() == OutIndices.Num() * OutIndices.GetTypeSize());
		FMemory::Memcpy(OutIndices.GetData(),IndexStorage.GetData(),IndexStorage.Num());
	}
	else
	{
		// Copy element by element promoting 16-bit integers to 32-bit.
		check(IndexStorage.Num() == OutIndices.Num() * sizeof(uint16));
		const uint16* SrcIndices16Bit = reinterpret_cast<const uint16*>(IndexStorage.GetData());
		for (uint32 i = 0; i < NumIndices; ++i)
		{
			OutIndices[i] = SrcIndices16Bit[i];
		}
	}
}

const uint16* FVoxelRawStaticIndexBuffer::AccessStream16() const
{
	if (!b32Bit)
	{
		return reinterpret_cast<const uint16*>(IndexStorage.GetData());
	}
	return nullptr;
}

FIndexArrayView FVoxelRawStaticIndexBuffer::GetArrayView() const
{
	return FIndexArrayView(IndexStorage.GetData(), NumIndices, b32Bit);
}

void FVoxelRawStaticIndexBuffer::InitRHI()
{
	const uint32 IndexStride = b32Bit ? sizeof(uint32) : sizeof(uint16);
	const uint32 SizeInBytes = IndexStorage.Num();
	check(NumIndices == (b32Bit ? (IndexStorage.Num() / 4) : (IndexStorage.Num() / 2)));

	if (SizeInBytes > 0)
	{
		// Create the index buffer.
		FRHIResourceCreateInfo CreateInfo(&IndexStorage);
		IndexBufferRHI = RHICreateIndexBuffer(IndexStride,SizeInBytes,BUF_Static,CreateInfo);
	}    
}

void FVoxelRawStaticIndexBuffer::Serialize(FArchive& Ar, bool bNeedsCPUAccess)
{
	IndexStorage.SetAllowCPUAccess(bNeedsCPUAccess);

	if (Ar.UE4Ver() < VER_UE4_SUPPORT_32BIT_STATIC_MESH_INDICES)
	{
		TResourceArray<uint16,INDEXBUFFER_ALIGNMENT> LegacyIndices;

		b32Bit = false;
		LegacyIndices.BulkSerialize(Ar);
		const int32 NumLegacyIndices = LegacyIndices.Num();
		const int32 IndexStride = sizeof(uint16);
		IndexStorage.Empty(NumLegacyIndices * IndexStride);
		IndexStorage.AddUninitialized(NumLegacyIndices * IndexStride);
		FMemory::Memcpy(IndexStorage.GetData(),LegacyIndices.GetData(),IndexStorage.Num());
	}
	else
	{
		Ar << b32Bit;
		IndexStorage.BulkSerialize(Ar);
	}
}

void FVoxelRawStaticIndexBuffer::Discard()
{
    IndexStorage.SetAllowCPUAccess(false);
    IndexStorage.Discard();

	UpdateCachedNumIndices();
}