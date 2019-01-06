// Copyright 2019 Phyronnaz

#include "VoxelMaterialCollectionFactory.h"
#include "VoxelMaterialCollection.h"
#include "AssetTypeCategories.h"

UVoxelMaterialCollectionFactory::UVoxelMaterialCollectionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelMaterialCollection::StaticClass();
}

UObject* UVoxelMaterialCollectionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewCollection = NewObject<UVoxelMaterialCollection>(InParent, Class, Name, Flags | RF_Transactional);

	return NewCollection;
}