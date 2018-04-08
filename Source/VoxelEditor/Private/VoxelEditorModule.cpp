// Copyright 2018 Phyronnaz

#include "VoxelEditorModule.h"
#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "PropertyEditorModule.h"

#include "VoxelWorldDetails.h"

#include "IAssetTools.h"
#include "AssetToolsModule.h"




#include "VoxelWorldGeneratorPickerCustomization.h"
#include "VoxelCrashReporterEditor.h"
#include "IPlacementModeModule.h"
#include "AssetData.h"
#include "SlateStyle.h"
#include "SlateStyleRegistry.h"
#include "ClassIconFinder.h"
#include "IPluginManager.h"
#include "VoxelWorld.h"

#define LOCTEXT_NAMESPACE "FVoxelEditorModule"

const FVector2D Icon14x14(14.0f, 14.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);
const FVector2D Icon512x512(512.0f, 512.0f);

/**
 * Implements the VoxelEditor module.
 */
class FVoxelEditorModule : public IVoxelEditorModule
{
public:
	//~ IModuleInterface interface

	virtual void StartupModule() override
	{
		// Register CrashReporter. TODO: Maybe should be done with a load module?
		FVoxelCrashReporter::CrashReporter = MakeShareable(new FVoxelCrashReporterEditor());

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		VoxelAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Voxel")), LOCTEXT("VoxelAssetCategory", "Voxel"));

		RegisterPlacementModeExtensions();
		RegisterClassLayout();
		RegisterAssetTools();


		// Icons
		{
			FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Voxel"))->GetContentDir() + "/";

			StyleSet = MakeShareable(new FSlateStyleSet("VoxelStyle"));
			StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
			StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

			// VoxelWorld
			StyleSet->Set("ClassThumbnail.VoxelWorld", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorld", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_16x.png"), Icon16x16));

			// Importers

			// Compile To C++
			StyleSet->Set("VoxelGraphEditor.CompileToCpp", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_compile_40x.png")), Icon40x40));
			StyleSet->Set("VoxelGraphEditor.CompileToCpp.Small", new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_compile_40x.png")), Icon20x20));

			// Actors
			StyleSet->Set("ClassThumbnail.VoxelActorSpawner", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/ActorSpawner_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelActorSpawner", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/ActorSpawner_16x.png"), Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelActorGroup", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/ActorGroup_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelActorGroup", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/ActorGroup_16x.png"), Icon16x16));
			
			// Grass
			StyleSet->Set("ClassThumbnail.VoxelGrassSpawner", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/GrassSpawner_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelGrassSpawner", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/GrassSpawner_16x.png"), Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelGrassGroup", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/GrassGroup_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelGrassGroup", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/GrassGroup_16x.png"), Icon16x16));

			// Voxel Graph
			StyleSet->Set("ClassThumbnail.VoxelGraphGenerator", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelGraph_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelGraphGenerator", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelGraph_16x.png"), Icon16x16));
			
			// Data Asset
			StyleSet->Set("ClassThumbnail.VoxelDataAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/DataAsset_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelDataAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/DataAsset_16x.png"), Icon16x16));

			// Landscape asset
			StyleSet->Set("ClassThumbnail.VoxelLandscapeAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Landscape_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelLandscapeAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Landscape_16x.png"), Icon16x16));

			// World generator
			StyleSet->Set("ClassThumbnail.VoxelWorldGenerator", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorldGenerator", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_16x.png"), Icon16x16));

			// Voxel Asset
			StyleSet->Set("ClassThumbnail.VoxelAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelAsset_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelAsset", new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelAsset_16x.png"), Icon16x16));
			
			FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
		}
	}

	virtual void ShutdownModule() override
	{
		UnregisterPlacementModeExtensions();
		UnregisterClassLayout();
		UnregisterAssetTools();

		if (StyleSet.IsValid())
		{
			FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
			StyleSet.Reset();
		}
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}


protected:
	
	/** Registers placement mode extensions. */
	void RegisterPlacementModeExtensions()
	{
		FPlacementCategoryInfo Info(
			LOCTEXT("VoxelCategoryName", "Voxel"),
			"Voxel",
			TEXT("PMVoxel"),
			25
		);

		IPlacementModeModule::Get().RegisterPlacementCategory(Info);
		IPlacementModeModule::Get().RegisterPlaceableItem(Info.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(AVoxelWorld::StaticClass()))));
	}

	/** Unregisters placement mode extensions. */
	void UnregisterPlacementModeExtensions()
	{
		if (IPlacementModeModule::IsAvailable())
		{
			IPlacementModeModule::Get().UnregisterPlacementCategory("Voxel");
		}
	}

	void RegisterClassLayout()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		//Custom detail views
		PropertyModule.RegisterCustomClassLayout("VoxelWorld", FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelWorldDetails::MakeInstance));

		PropertyModule.RegisterCustomPropertyTypeLayout("VoxelWorldGeneratorPicker", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVoxelWorldGeneratorPickerCustomization::MakeInstance));

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	void UnregisterClassLayout()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule != nullptr)
		{
			PropertyModule->UnregisterCustomClassLayout("VoxelWorld");
			PropertyModule->UnregisterCustomClassLayout("VoxelLandscapeImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelHeightmapImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelSplineImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelMeshImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelRawVoxImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelMagicaVoxelImporter");
			PropertyModule->UnregisterCustomClassLayout("VoxelWorldSectionImporter");
			PropertyModule->NotifyCustomizationModuleChanged();
		}
	}

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		
	}

	/**
	* Registers a single asset type action.
	*
	* @param AssetTools The asset tools object to register with.
	* @param Action The asset type action to register.
	*/
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	/** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}


private:
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	EAssetTypeCategories::Type VoxelAssetCategoryBit;
	TSharedPtr< class FSlateStyleSet > StyleSet;
};


IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);


#undef LOCTEXT_NAMESPACE
#undef IMAGE_PLUGIN_BRUSH
