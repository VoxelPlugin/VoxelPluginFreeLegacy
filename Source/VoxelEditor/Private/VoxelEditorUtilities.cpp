// Copyright 2021 Phyronnaz

#include "VoxelEditorUtilities.h"
#include "VoxelMinimal.h"

#include "AssetToolsModule.h"
#include "AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

UObject* FVoxelEditorUtilities::CreateAsset(FString AssetName, FString FolderPath, UClass* Class, FString Suffix)
{
	FString PackageName = FolderPath / AssetName;

	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(PackageName, Suffix, PackageName, AssetName);

#if VOXEL_ENGINE_VERSION < 426
	UPackage* Package = CreatePackage(nullptr, *PackageName);
#else
	UPackage* Package = CreatePackage(*PackageName);
#endif

	if (!ensure(Package))
	{
		return nullptr;
	}

	auto* Object = NewObject<UObject>(Package, Class, *AssetName, RF_Public | RF_Standalone);
	if (!ensure(Object))
	{
		return nullptr;
	}
	
	Object->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Object);

	return Object;
}