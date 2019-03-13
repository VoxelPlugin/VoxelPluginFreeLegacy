// Copyright 2019 Phyronnaz

#include "VoxelWorldSaveObjectFactory.h"
#include "VoxelData/VoxelSave.h"
#include "AssetTypeCategories.h"

UVoxelWorldSaveObjectFactory::UVoxelWorldSaveObjectFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UVoxelWorldSaveObject::StaticClass();
}

UObject* UVoxelWorldSaveObjectFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UVoxelWorldSaveObject>(InParent, Class, Name, Flags);
}