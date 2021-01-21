// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSaveStruct.h"
#include "VoxelFoliageSave.generated.h"

namespace FVoxelFoliageSaveVersion
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

struct VOXELFOLIAGE_API FVoxelFoliageSaveImpl
{	
	FVoxelFoliageSaveImpl() = default;

	bool Serialize(FArchive& Ar)
	{
		if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
		{
			if (Ar.IsSaving())
			{
				Version = FVoxelFoliageSaveVersion::LatestVersion;
			}

			Ar << Version;
			Ar << Guid;
			Ar << CompressedData;
		}

		return true;
	}

	bool operator==(const FVoxelFoliageSaveImpl& Other) const
	{
		return Guid == Other.Guid;
	}
	
private:
	// Version of FVoxelFoliageSave, not of the compressed data!
	int32 Version;
	FGuid Guid;
	TArray<uint8> CompressedData;

	friend class FVoxelFoliageSubsystem;
};

USTRUCT(BlueprintType, Category = Voxel)
struct VOXELFOLIAGE_API FVoxelFoliageSave
#if CPP
	: public TVoxelSaveStruct<FVoxelFoliageSaveImpl>
#endif
{	
	GENERATED_BODY()
};

DEFINE_VOXEL_SAVE_STRUCT(FVoxelFoliageSave)