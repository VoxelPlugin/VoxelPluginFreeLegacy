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

namespace FVoxelSerializationUtilities
{
	VOXEL_API void SerializeValues(FArchive& Archive, TNoGrowArray<FVoxelValue>& Values, uint32 ValueConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion);
	VOXEL_API void SerializeMaterials(FArchive& Archive, TNoGrowArray<FVoxelMaterial>& Materials, uint32 MaterialConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion);

	template<typename T>
	void SerializeMaterials(FArchive& Archive, TNoGrowArray<TVoxelMaterialStorage<T>>& Materials, uint32 MaterialConfigFlag)
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

	//////////////////////////////////////////////////////////////////////////////

	VOXEL_API void CompressData(
		const uint8* UncompressedData,
		int64 UncompressedDataNum, 
		TArray<uint8>& OutCompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault);
	VOXEL_API void CompressData(
		FLargeMemoryWriter& UncompressedData,
		TArray<uint8>& CompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault);
	
	inline void CompressData(
		const TArray<uint8>& UncompressedData, 
		TArray<uint8>& CompressedData,
		EVoxelCompressionLevel::Type CompressionLevel = EVoxelCompressionLevel::VoxelDefault)
	{
		CompressData(UncompressedData.GetData(), UncompressedData.Num(), CompressedData, CompressionLevel);
	}

	VOXEL_API bool DecompressData(const TArray<uint8>& CompressedData, TArray64<uint8>& UncompressedData);

	VOXEL_API void TestCompression(int64 Size, EVoxelCompressionLevel::Type CompressionLevel);
}