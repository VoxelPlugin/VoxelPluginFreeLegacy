// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"

namespace FVoxelSerializationUtilities
{
	inline void AddMaterialsToArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive)
	{
		int MaterialsSize = Materials.Num();
		Archive << MaterialsSize;
		for (int I = 0; I < MaterialsSize; I++)
		{
			Archive << Materials[I];
		}
	}

	inline void GetMaterialsFromArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 ConfigFlags)
	{
		if (ConfigFlags == GetVoxelConfigFlags())
		{
			int MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int I = 0; I < MaterialsSize; I++)
			{
				Archive << Materials[I];
			}
		}
		else
		{
			int MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int I = 0; I < MaterialsSize; I++)
			{
				Materials[I] = FVoxelMaterial::SerializeCompat(Archive, ConfigFlags);
			}
		}
	}

	inline void SerializeMaterials(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 ConfigFlags)
	{
		if (Archive.IsLoading())
		{
			GetMaterialsFromArchive(Materials, Archive, ConfigFlags);
		}
		else if (Archive.IsSaving())
		{
			AddMaterialsToArchive(Materials, Archive);
		}
	}

	inline void CompressData(const TArray<uint8>& UncompressedData, TArray<uint8>& CompressedData, ECompressionFlags CompressionFlags = (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed))
	{
		int32 UncompressedSize = UncompressedData.Num();
		int32 CompressedSize = FCompression::CompressMemoryBound(CompressionFlags, UncompressedSize);

		CompressedData.SetNumUninitialized(CompressedSize + sizeof(UncompressedSize));

		FMemory::Memcpy(&CompressedData[0], &UncompressedSize, sizeof(UncompressedSize));
		verify(FCompression::CompressMemory(CompressionFlags, CompressedData.GetData() + sizeof(UncompressedSize), CompressedSize, UncompressedData.GetData(), UncompressedData.Num()));
		CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));

		CompressedData.Add(CompressionFlags);
	}

	inline bool DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData)
	{
		if (CompressedData.Num() == 0)
		{
			return false;
		}

		ECompressionFlags CompressionFlags = (ECompressionFlags)CompressedData.Last();

		int32 UncompressedSize;
		FMemory::Memcpy(&UncompressedSize, &CompressedData[0], sizeof(UncompressedSize));
		UncompressedData.SetNum(UncompressedSize);
		return FCompression::UncompressMemory(CompressionFlags, UncompressedData.GetData(), UncompressedSize, CompressedData.GetData() + sizeof(UncompressedSize), CompressedData.Num() - 1 - sizeof(UncompressedSize));
	}
}