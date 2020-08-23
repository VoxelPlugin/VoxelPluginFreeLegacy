// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_VoxelBase : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_VoxelBase(EAssetTypeCategories::Type AssetCategory)
		: AssetCategory(AssetCategory)
	{
	}

	virtual uint32 GetCategories() override { return AssetCategory; }
	
private:
	EAssetTypeCategories::Type AssetCategory;
};

