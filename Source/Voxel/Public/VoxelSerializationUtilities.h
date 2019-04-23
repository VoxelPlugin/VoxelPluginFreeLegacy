// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Compression.h"

struct FVoxelMaterial;
class FArchive;

namespace FVoxelSerializationUtilities
{
	void AddMaterialsToArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive);
	void GetMaterialsFromArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);

	void SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial>& Materials, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);

	void CompressData(const TArray<uint8>& UncompressedData, TArray<uint8>& CompressedData, ECompressionFlags CompressionFlags = (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed));
	bool DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData);
}