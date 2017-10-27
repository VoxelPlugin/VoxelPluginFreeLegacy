// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "VoxelDataAsset.h"
#include "Engine/World.h"
#include "BufferArchive.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "MemoryReader.h"
#include "Engine/Texture2D.h"


UVoxelDataAsset::UVoxelDataAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
};

void UVoxelDataAsset::Init(FDecompressedVoxelDataAsset& Asset)
{
	FBufferArchive ToBinary;

	ToBinary << Asset;

	Data.Empty();
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	// Send entire binary array/archive to compressor
	Compressor << ToBinary;

	// Send archive serialized data to binary array
	Compressor.Flush();
}

bool UVoxelDataAsset::GetDecompressedAsset(FDecompressedVoxelDataAsset& Asset)
{
	if (Data.Num() != 0)
	{
		FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

		check(!Decompressor.GetError());

		//Decompress
		FBufferArchive DecompressedBinaryArray;
		Decompressor << DecompressedBinaryArray;

		FMemoryReader FromBinary = FMemoryReader(DecompressedBinaryArray);
		FromBinary.Seek(0);

		FromBinary << Asset;

		check(FromBinary.AtEnd());

		return true;
	}
	else
	{
		return false;
	}
}

FORCEINLINE void FDecompressedVoxelDataAsset::SetSize(int32 NewSizeX, int32 NewSizeY, int32 NewSizeZ)
{
	SizeX = NewSizeX;
	SizeY = NewSizeY;
	SizeZ = NewSizeZ;

	int Count = SizeX * SizeY * SizeZ;

	Values.SetNumUninitialized(Count);
	Materials.SetNumUninitialized(Count);
	VoxelTypes.SetNumUninitialized(Count);
}

float FDecompressedVoxelDataAsset::GetValue(const int X, const int Y, const int Z)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	return Values[X + SizeX * Y + SizeX * SizeY * Z];
}

FVoxelMaterial FDecompressedVoxelDataAsset::GetMaterial(const int X, const int Y, const int Z)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	return Materials[X + SizeX * Y + SizeX * SizeY * Z];
}

EVoxelType FDecompressedVoxelDataAsset::GetVoxelType(const int X, const int Y, const int Z)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	return (EVoxelType)VoxelTypes[X + SizeX * Y + SizeX * SizeY * Z];
}

void FDecompressedVoxelDataAsset::SetValue(const int X, const int Y, const int Z, const float NewValue)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	Values[X + SizeX * Y + SizeX * SizeY * Z] = NewValue;
}

void FDecompressedVoxelDataAsset::SetMaterial(const int X, const int Y, const int Z, const FVoxelMaterial NewMaterial)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	Materials[X + SizeX * Y + SizeX * SizeY * Z] = NewMaterial;
}

void FDecompressedVoxelDataAsset::SetVoxelType(const int X, const int Y, const int Z, const EVoxelType VoxelType)
{
	check(0 <= X && X < SizeX);
	check(0 <= Y && Y < SizeY);
	check(0 <= Z && Z < SizeZ);
	VoxelTypes[X + SizeX * Y + SizeX * SizeY * Z] = VoxelType;
}
