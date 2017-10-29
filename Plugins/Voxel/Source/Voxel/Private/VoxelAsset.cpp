// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelAsset.h"

#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"


UVoxelAsset::UVoxelAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
};

UVoxelAsset::~UVoxelAsset()
{

}

void UVoxelAsset::InitFromAsset(FDecompressedVoxelAsset* Asset)
{
	check(Asset);

	FBufferArchive ToBinary;

	AddAssetToArchive(ToBinary, Asset);

	Data.Empty();
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	// Send entire binary array/archive to compressor
	Compressor << ToBinary;

	// Send archive serialized data to binary array
	Compressor.Flush();
}

bool UVoxelAsset::GetDecompressedAsset(FDecompressedVoxelAsset*& Asset, const float VoxelSize)
{
	check(Asset);

	if (Data.Num() != 0)
	{
		FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

		check(!Decompressor.GetError());

		//Decompress
		FBufferArchive DecompressedBinaryArray;
		Decompressor << DecompressedBinaryArray;

		FMemoryReader FromBinary = FMemoryReader(DecompressedBinaryArray);
		FromBinary.Seek(0);

		GetAssetFromArchive(FromBinary, Asset);

		check(FromBinary.AtEnd());

		return true;
	}
	else
	{
		return false;
	}
}

void UVoxelAsset::AddAssetToArchive(FBufferArchive& ToBinary, FDecompressedVoxelAsset* Asset)
{
	// TODO: doc
	check(false);
}

void UVoxelAsset::GetAssetFromArchive(FMemoryReader& FromBinary, FDecompressedVoxelAsset* Asset)
{
	// TODO: doc
	check(false);
}