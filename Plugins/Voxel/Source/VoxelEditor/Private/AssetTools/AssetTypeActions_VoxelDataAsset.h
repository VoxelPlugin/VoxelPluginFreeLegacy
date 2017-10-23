// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelDataAsset.h"

class FAssetTypeActions_VoxelDataAsset : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelDataAsset", "Voxel Data Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 0, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelDataAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
};
