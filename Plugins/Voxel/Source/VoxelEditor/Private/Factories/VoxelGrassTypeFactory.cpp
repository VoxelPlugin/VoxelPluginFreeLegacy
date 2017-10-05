// Copyright 2017 Phyronnaz

#include "VoxelGrassTypeFactory.h"
#include "AssetTypeCategories.h"
#include "VoxelGrassType.h"

UVoxelGrassTypeFactory::UVoxelGrassTypeFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelGrassType::StaticClass();
}

UObject* UVoxelGrassTypeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewGrassType = NewObject<UVoxelGrassType>(InParent, Class, Name, Flags | RF_Transactional);

	return NewGrassType;
}

uint32 UVoxelGrassTypeFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}
