// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSaveStruct.h"
#include "VoxelSpawnerSave.generated.h"

namespace FVoxelSpawnersSaveVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		SHARED_RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		SHARED_ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		SHARED_NoVoxelMaterialInHeightmapAssets,
		SHARED_FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		SpawnerRefactor,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
}

struct VOXEL_API FVoxelSpawnersSaveImpl
{	
	FVoxelSpawnersSaveImpl() = default;

	bool Serialize(FArchive& Ar)
	{
		if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
		{
			if (Ar.IsSaving())
			{
				Version = FVoxelSpawnersSaveVersion::LatestVersion;
			}

			Ar << Version;
			Ar << Guid;
			Ar << CompressedData;
		}

		return true;
	}

	bool operator==(const FVoxelSpawnersSaveImpl& Other) const
	{
		return Guid == Other.Guid;
	}
	
private:
	// Version of FVoxelSpawnerSave, not of the compressed data!
	int32 Version;
	FGuid Guid;
	TArray<uint8> CompressedData;

	friend class FVoxelSpawnerManager;
};

USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelSpawnersSave
#if CPP
	: public TVoxelSaveStruct<FVoxelSpawnersSaveImpl>
#endif
{	
	GENERATED_BODY()
};

DEFINE_VOXEL_SAVE_STRUCT(FVoxelSpawnersSave)