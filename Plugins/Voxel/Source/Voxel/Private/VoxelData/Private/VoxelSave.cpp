#pragma once

#include "VoxelPrivatePCH.h"
#include "VoxelSave.h"
#include "BufferArchive.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "MemoryReader.h"



FVoxelChunkSave::FVoxelChunkSave()
	: Id(-1)
{

}

FVoxelChunkSave::FVoxelChunkSave(uint64 Id, FIntVector Position, TArray<float, TFixedAllocator<16 * 16 * 16>> Values, TArray<FVoxelMaterial, TFixedAllocator<16 * 16 * 16>>& Materials)
	: Id(Id)
	, Values(Values)
	, Materials(Materials)
{
}

FVoxelWorldSave::FVoxelWorldSave()
	: Depth(-1)
{

}

void FVoxelWorldSave::Init(int NewDepth, std::list<TSharedRef<FVoxelChunkSave>> ChunksList)
{
	Depth = NewDepth;

	FBufferArchive ToBinary;

	// Order matters
	for (auto Chunk : ChunksList)
	{
		ToBinary << *Chunk;
	}

	Data.Empty();
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	// Send entire binary array/archive to compressor
	Compressor << ToBinary;

	// Send archive serialized data to binary array
	Compressor.Flush();
}

std::list<FVoxelChunkSave> FVoxelWorldSave::GetChunksList()
{
	std::list<FVoxelChunkSave> ChunksList;

	FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	check(!Decompressor.GetError());

	//Decompress
	FBufferArchive DecompressedBinaryArray;
	Decompressor << DecompressedBinaryArray;

	FMemoryReader FromBinary = FMemoryReader(DecompressedBinaryArray);
	FromBinary.Seek(0);

	while (!FromBinary.AtEnd())
	{
		FVoxelChunkSave Chunk;
		FromBinary << Chunk;

		// Order matters
		ChunksList.push_back(Chunk);
	}

	return ChunksList;
}

FVoxelValueDiff::FVoxelValueDiff()
	: Id(-1)
	, Index(-1)
	, Value(0)
{

}

FVoxelValueDiff::FVoxelValueDiff(uint64 Id, int Index, float Value)
	: Id(Id)
	, Index(Index)
	, Value(Value)
{

}

FVoxelMaterialDiff::FVoxelMaterialDiff()
	: Id(-1)
	, Index(-1)
	, Material()
{

}

FVoxelMaterialDiff::FVoxelMaterialDiff(uint64 Id, int Index, FVoxelMaterial Material)
	: Id(Id)
	, Index(Index)
	, Material(Material)
{

}
