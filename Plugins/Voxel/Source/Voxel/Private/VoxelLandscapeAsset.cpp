// Copyright 2017 Phyronnaz

#pragma once
#include "VoxelPrivatePCH.h"
#include "LandscapeComponent.h"
#include "VoxelLandscapeAsset.h"
#include "Engine/World.h"
#include "BufferArchive.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "MemoryReader.h"
#include "Engine/Texture2D.h"


UVoxelLandscapeAsset::UVoxelLandscapeAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Precision(1)
	, HardnessMultiplier(1)
{
};

void UVoxelLandscapeAsset::Init(TArray<float>& Heights, TArray<FVoxelMaterial>& Materials, int Size)
{
	FDecompressedVoxelLandscapeAsset Asset;
	Asset.Heights = Heights;
	Asset.Materials = Materials;
	Asset.Size = Size;

	FBufferArchive ToBinary;

	ToBinary << Asset;

	Data.Empty();
	FArchiveSaveCompressedProxy Compressor = FArchiveSaveCompressedProxy(Data, ECompressionFlags::COMPRESS_ZLIB);

	// Send entire binary array/archive to compressor
	Compressor << ToBinary;

	// Send archive serialized data to binary array
	Compressor.Flush();
}

bool UVoxelLandscapeAsset::GetDecompressedAsset(FDecompressedVoxelLandscapeAsset& Asset)
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


		Asset.Precision = Precision;
		Asset.HardnessMultiplier = HardnessMultiplier;

		return true;
	}
	else
	{
		return false;
	}
}

float FDecompressedVoxelLandscapeAsset::GetValue(const int X, const int Y, const int Z, const float VoxelSize)
{
	check(0 <= X && X < Size && 0 <= Y && Y < Size);

	if ((Z + Precision) * VoxelSize < Heights[X + Size * Y])
	{
		// If voxel over us is in, we're entirely in
		return -HardnessMultiplier;
	}
	else if ((Z - Precision) * VoxelSize > Heights[X + Size * Y])
	{
		// If voxel under us is out, we're entirely out
		return HardnessMultiplier;
	}
	else
	{
		float Alpha = (Z * VoxelSize - Heights[X + Size * Y]) / VoxelSize / Precision;

		return Alpha * HardnessMultiplier;
	}
}

FVoxelMaterial FDecompressedVoxelLandscapeAsset::GetMaterial(const int X, const int Y, const int Z, const float VoxelSize)
{
	check(0 <= X && X < Size && 0 <= Y && Y < Size);

	return Materials[X + Size * Y];
}

int FDecompressedVoxelLandscapeAsset::GetLowerBound(int X, int Y, const float VoxelSize)
{
	return FMath::FloorToInt(Heights[X + Size * Y] / (float)VoxelSize - Precision);
}

int FDecompressedVoxelLandscapeAsset::GetUpperBound(int X, int Y, const float VoxelSize)
{
	return FMath::CeilToInt(Heights[X + Size * Y] / (float)VoxelSize + Precision);
}
