// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeActions_VoxelBase : public FAssetTypeActions_Base
{
public:
	explicit FAssetTypeActions_VoxelBase(EAssetTypeCategories::Type AssetCategory)
		: AssetCategory(AssetCategory)
	{
	}

	virtual uint32 GetCategories() override { return AssetCategory; }
	
private:
	EAssetTypeCategories::Type AssetCategory;
};

class FAssetTypeActions_Voxel : public FAssetTypeActions_VoxelBase
{
public:
	const FText Name;
	const FColor Color;
	UClass* const SupportedClass;

	FAssetTypeActions_Voxel(EAssetTypeCategories::Type AssetCategory, const FText& Name, FColor Color, UClass* SupportedClass)
		: FAssetTypeActions_VoxelBase(AssetCategory)
		, Name(Name)
		, Color(Color)
		, SupportedClass(SupportedClass)
	{
	}

	virtual FText GetName() const override { return Name; }
	virtual FColor GetTypeColor() const override { return Color; }
	virtual UClass* GetSupportedClass() const override { return SupportedClass; }
};