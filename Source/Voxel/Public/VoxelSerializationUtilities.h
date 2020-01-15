// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Compression.h"
#include "VoxelValue.h"

struct FVoxelMaterial;
class FArchive;

namespace FVoxelSerializationUtilities
{
	VOXEL_API void SerializeValues(FArchive& Archive, TArray<FVoxelValue>& Values, uint32 ValueConfigFlag, int32 VoxelCustomVersion);
	VOXEL_API void SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial>& Materials, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);

	VOXEL_API void CompressData(
		const uint8* UncompressedData, 
		int32 UncompressedDataNum, 
		TArray<uint8>& CompressedData, 
		ECompressionFlags CompressionFlags = ECompressionFlags(COMPRESS_ZLIB | COMPRESS_BiasSpeed));
	inline void CompressData(
		const TArray<uint8>& UncompressedData, 
		TArray<uint8>& CompressedData, 
		ECompressionFlags CompressionFlags = ECompressionFlags(COMPRESS_ZLIB | COMPRESS_BiasSpeed))
	{
		CompressData(UncompressedData.GetData(), UncompressedData.Num(), CompressedData, CompressionFlags);
	}

	VOXEL_API bool DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData);
}