// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"

struct FVoxelCustomVersion
{
	enum Type
	{
		BeforeCustomVersionWasAdded,
		PlaceableItemsInSave,
		AssetItemsImportValueMaterials,
		DataAssetScale,
		RemoveVoxelGrass,

		// -----<new versions can be added above this line>-------------------------------------------------
		VoxelVersionPlusOne,
		LatestVersion = VoxelVersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FVoxelCustomVersion() {}
};