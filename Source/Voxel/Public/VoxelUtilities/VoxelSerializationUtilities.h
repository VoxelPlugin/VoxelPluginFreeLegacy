// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

class FArchive;
class FLargeMemoryWriter;

namespace FVoxelSerializationVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		SHARED_NoVoxelMaterialInHeightmapAssets,
		SHARED_FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		SHARED_StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

namespace EVoxelCompressionLevel
{
	enum Type : int32
	{
		NoCompression = 0,
		BestSpeed = 1,
		BestCompression = 9,
		DefaultCompression = -1,
		VoxelDefault = -2
	};
}

struct VOXEL_API FVoxelSerializationUtilities
{
public:
	template<typename TAllocator>
	static void SerializeValues(FArchive& Archive, TArray<FVoxelValue, TAllocator>& Values, uint32 ValueConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion);
	template<typename TAllocator>
	static void SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial, TAllocator>& Materials, uint32 MaterialConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion);

	template<typename T, typename TAllocator>
	static void SerializeMaterials(FArchive& Archive, TArray<TVoxelMaterialStorage<T>, TAllocator>& Materials, uint32 MaterialConfigFlag)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();

		static_assert(sizeof(TVoxelMaterialStorage<T>) / sizeof(T) == TVoxelMaterialStorage<T>::NumChannels, "Serialization below will be broken");
		
		if (Archive.IsLoading())
		{
			if (MaterialConfigFlag == GVoxelMaterialConfigFlag)
			{
				int32 MaterialsSize;
				Archive << MaterialsSize;
				Materials.Empty(MaterialsSize);
				Materials.SetNumUninitialized(MaterialsSize);
				Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(TVoxelMaterialStorage<T>));
			}
			else
			{
				int32 MaterialsSize;
				Archive << MaterialsSize;
				Materials.Empty(MaterialsSize);
				Materials.SetNumUninitialized(MaterialsSize);
				for (int32 I = 0; I < MaterialsSize; I++)
				{
					Materials[I] = TVoxelMaterialStorage<T>::SerializeWithCustomConfig(Archive, MaterialConfigFlag);
				}
			}
		}
		else if (Archive.IsSaving())
		{
			int32 MaterialsSize = Materials.Num();
			Archive << MaterialsSize;
			Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(TVoxelMaterialStorage<T>));
		}
	}

public:
	static void CompressData(
		const uint8* UncompressedData,
		int64 UncompressedDataNum, 
		TArray<uint8>& OutCompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault);
	static void CompressData(
		FLargeMemoryWriter& UncompressedData,
		TArray<uint8>& CompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault);
	
	static void CompressData(
		const TArray<uint8>& UncompressedData, 
		TArray<uint8>& CompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault)
	{
		CompressData(UncompressedData.GetData(), UncompressedData.Num(), CompressedData, CompressionLevel);
	}

	static bool DecompressData(const TArray<uint8>& CompressedData, TArray64<uint8>& UncompressedData);
	static void TestCompression(int64 Size, EVoxelCompressionLevel::Type CompressionLevel);

private:
	static constexpr int64 MaxChunkSize = MAX_int32; // Could be uint32, but let's not take any risk of overflow
	static constexpr int64 MaxNumChunks = 16; // That's 32GB
	
	struct FHeader
	{
		// Need to store a special flag to tell DecompressData this is a 64 bit archive following the new format
		const int32 LegacyFlag = -1;
		// Sanity check
		const uint32 Magic = 0xDEADBEEF;
		
		// Sanity check
		int64 CompressedSize = 0;
		// To pre-allocate buffer
		int64 UncompressedSize = 0;
		
		uint32 Flags = 0;
		uint32 NumChunks = 0;
		
		TVoxelStaticArray<uint32, MaxNumChunks> ChunksCompressedSize{ ForceInit };
	};
	static_assert(sizeof(FHeader) == 4 + 4 + 8 + 8 + 4 + 4 + MaxNumChunks * 4, "");
};