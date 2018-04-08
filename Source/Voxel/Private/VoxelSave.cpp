// Copyright 2018 Phyronnaz

#include "VoxelSave.h"
#include "BufferArchive.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "MemoryReader.h"



FVoxelChunkSave::FVoxelChunkSave()
	: Id(-1)
{

}

FVoxelChunkSave::FVoxelChunkSave(uint64 Id, FIntVector Position, float InValues[DATA_CHUNK_TOTAL_SIZE], FVoxelMaterial InMaterials[DATA_CHUNK_TOTAL_SIZE])
	: Id(Id)
{
	Values.SetNumUninitialized(DATA_CHUNK_TOTAL_SIZE);
	Materials.SetNumUninitialized(DATA_CHUNK_TOTAL_SIZE);

	for (int Index = 0; Index < DATA_CHUNK_TOTAL_SIZE; Index++)
	{
		Values[Index] = InValues[Index];
		Materials[Index] = InMaterials[Index];
	}
}

FVoxelWorldSave::FVoxelWorldSave()
	: LOD(-1)
{

}

void FVoxelWorldSave::Init(int NewLOD, const TArray<FVoxelChunkSave>& ChunksList)
{
	LOD = NewLOD;

	FBufferArchive ToBinary;

	// Order matters
	for (auto Chunk : ChunksList)
	{
		ToBinary << Chunk;
	}

	Data.Empty();
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	// Send entire binary array/archive to compressor
	Compressor << ToBinary;

	// Send archive serialized data to binary array
	Compressor.Flush();
}

void FVoxelWorldSave::GetChunksQueue(TArray<FVoxelChunkSave>& SaveQueue) const
{
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

		SaveQueue.Add(Chunk);
	}
}
