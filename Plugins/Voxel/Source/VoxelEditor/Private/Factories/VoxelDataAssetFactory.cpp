// Copyright 2017 Phyronnaz

#include "VoxelDataAssetFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelDataAsset.h"

UVoxelDataAssetFactory::UVoxelDataAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = false;
	bEditAfterNew = true;
	SupportedClass = UVoxelDataAsset::StaticClass();
}

UObject* UVoxelDataAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelDataAsset>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}

uint32 UVoxelDataAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
