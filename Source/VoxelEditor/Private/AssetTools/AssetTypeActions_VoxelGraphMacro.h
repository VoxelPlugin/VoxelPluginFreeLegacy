// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_VoxelBase.h"

class FMenuBuilder;

class FAssetTypeActions_VoxelGraphMacro : public FAssetTypeActions_VoxelBase
{
public:
	using FAssetTypeActions_VoxelBase::FAssetTypeActions_VoxelBase;

	virtual FText GetName() const override { return VOXEL_LOCTEXT("Voxel Graph Macro"); }
	virtual FColor GetTypeColor() const override { return FColor(0, 175, 255); }
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
};