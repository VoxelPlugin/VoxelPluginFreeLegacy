// Copyright 2020 Phyronnaz

#include "AssetTools/AssetTypeActions_VoxelFoliageBiomeType.h"
#include "Factories/VoxelFactories.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ToolMenuSection.h"

void FAssetTypeActions_VoxelFoliageBiomeType::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	if (InObjects.Num() != 1)
	{
		return;
	}
	
	Section.AddMenuEntry(
		"VoxelFoliage_NewBiome",
		VOXEL_LOCTEXT("Create biome instance"),
		VOXEL_LOCTEXT("Creates a new biome using this as type"),
		{},
		FUIAction(FExecuteAction::CreateLambda([this, Objects = GetTypedWeakObjectPtrs<UVoxelFoliageBiomeType>(InObjects)]()
		{
			UVoxelFoliageBiomeType* Object = Objects[0].Get();
			if (!Object)
			{
				return;
			}
			
			// Create an appropriate and unique name 
			FString Name;
			FString PackageName;
			CreateUniqueAssetName(Object->GetOutermost()->GetName(), "_Inst", PackageName, Name);

			auto* Factory = NewObject<UVoxelFoliageBiomeFactory>();
			Factory->Type = Object;

			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackageName), UVoxelFoliageBiome::StaticClass(), Factory);
		})));
}