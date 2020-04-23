// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

struct FVoxelCustomVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		PlaceableItemsInSave,
		AssetItemsImportValueMaterials,
		DataAssetScale,
		RemoveVoxelGrass,
		DataAssetTransform,
		RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		FoliagePaint,
		ValueConfigFlagAndSaveGUIDs,
		SingleValues,
		NoVoxelMaterialInHeightmapAssets,
		FixMissingMaterialsInHeightmapAssets,
		AddUserFlagsToSaves,
		StoreSpawnerMatricesRelativeToComponent,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VoxelVersionPlusOne,
		LatestVersion = VoxelVersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FVoxelCustomVersion() {}
};