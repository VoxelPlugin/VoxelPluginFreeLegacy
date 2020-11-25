// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelImporters/VoxelMagicaVoxImporter.h"

class FAssetTypeActions_VoxelMagicaVoxScene : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Magica Vox Scene"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 140, 0); }
	virtual UClass* GetSupportedClass() const override { return UVoxelMagicaVoxScene::StaticClass(); }
};