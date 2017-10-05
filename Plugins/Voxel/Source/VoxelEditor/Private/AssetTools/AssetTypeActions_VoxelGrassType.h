// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "VoxelGrassType.h"

class FAssetTypeActions_VoxelGrassType : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_VoxelGrassType", "Voxel Grass Type"); }
	virtual FColor GetTypeColor() const override { return FColor(128, 255, 128); }
	virtual UClass* GetSupportedClass() const override { return UVoxelGrassType::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
};
