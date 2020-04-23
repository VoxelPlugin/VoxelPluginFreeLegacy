// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_VoxelBase.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"

class FAssetTypeActions_VoxelHeightmapAsset : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Heightmap Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(200, 80, 80); }
	virtual UClass* GetSupportedClass() const override { return UVoxelHeightmapAsset::StaticClass(); }
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

private:
	/** Can we execute a reimport for the selected objects? */
	bool CanExecuteReimport(const TArray<TWeakObjectPtr<UVoxelHeightmapAsset>> Objects) const;

	/** Handler for when Reimport is selected */
	void ExecuteReimport(const TArray<TWeakObjectPtr<UVoxelHeightmapAsset>> Objects) const;
};