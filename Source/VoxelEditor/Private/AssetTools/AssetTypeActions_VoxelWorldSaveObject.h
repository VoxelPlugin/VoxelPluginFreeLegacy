// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelSave.h"

class FAssetTypeActions_VoxelWorldSaveObject : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelWorldSaveObject(EAssetTypeCategories::Type InAssetCategory)
		: MyAssetCategory(InAssetCategory)
	{

	}

	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelWorldSaveObject", "Voxel World Save Object"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 140, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelWorldSaveObject::StaticClass(); }
	virtual uint32 GetCategories() override { return MyAssetCategory; }

private:
	EAssetTypeCategories::Type MyAssetCategory;
};