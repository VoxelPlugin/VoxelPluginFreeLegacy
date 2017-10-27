// Copyright 2017 Phyronnaz

#include "VoxelLandscapeAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelLandscapeAsset.h"

UVoxelLandscapeAssetFactory::UVoxelLandscapeAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelLandscapeAsset::StaticClass();
}

UObject* UVoxelLandscapeAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewLandscapeAsset = NewObject<UVoxelLandscapeAsset>(InParent, Class, Name, Flags);

	return NewLandscapeAsset;
}

uint32 UVoxelLandscapeAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
