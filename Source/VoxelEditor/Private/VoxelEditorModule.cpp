// Copyright 2018 Phyronnaz

#include "VoxelEditorModule.h"

#include "Interfaces/IPluginManager.h"
#include "IPlacementModeModule.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "EditorModeRegistry.h"

#include "VoxelWorld.h"

#include "VoxelCrashReporterEditor.h"

#include "AssetTools/AssetTypeActions_VoxelWorldSaveObject.h"
#include "AssetTools/AssetTypeActions_VoxelMaterialCollection.h"






#include "Factories/VoxelWorldFactory.h"

#include "Details/VoxelWorldDetails.h"
#include "Details/VoxelWorldGeneratorPickerCustomization.h"
#include "Details/VoxelMaterialCollectionDetails.h"
#include "Styling/SlateStyleRegistry.h"


#define LOCTEXT_NAMESPACE "VoxelEditorModule"

const FVector2D Icon14x14(14.0f, 14.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);
const FVector2D Icon512x512(512.0f, 512.0f);

DEFINE_LOG_CATEGORY(LogVoxelEditor);

/**
 * Implements the VoxelEditor module.
 */
class FVoxelEditorModule : public IVoxelEditorModule
{
public:
	virtual void StartupModule() override
	{
		FVoxelCrashReporterEditor::Register();

		// Voxel asset category
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
			StyleSet->Set("ClassThumbnail.VoxelWorld"                        , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorld"                             , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_16x.png"), Icon16x16));
																		     
			// Voxel Material Collection								     
			StyleSet->Set("ClassThumbnail.VoxelMaterialCollection"           , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/AssetIcons/PaperTileMap_64x.png")), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelMaterialCollection"                , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/AssetIcons/PaperTileMap_16x.png")), Icon16x16));
																		     
																		     
			// World generator											     
			StyleSet->Set("ClassThumbnail.VoxelWorldGenerator"               , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorldGenerator"                    , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_16x.png"), Icon16x16));
																		     
			// Voxel World Object Save									     
			StyleSet->Set("ClassThumbnail.VoxelWorldSaveObject"              , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelWorldSaveObject_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorldSaveObject"                   , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelWorldSaveObject_16x.png"), Icon16x16));
			
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
		IPlacementModeModule::Get().RegisterPlaceableItem(Info.UniqueHandle, MakeShareable(new FPlaceableItem(UVoxelWorldFactory::StaticClass()->GetDefaultObject<UVoxelWorldFactory>(), FAssetData(AVoxelWorld::StaticClass()))));
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
		PropertyModule.RegisterCustomClassLayout("VoxelWorld"                    , FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelWorldDetails                    ::MakeInstance));
		PropertyModule.RegisterCustomClassLayout("VoxelMaterialCollection"       , FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelMaterialCollectionDetails       ::MakeInstance));
		
		PropertyModule.RegisterCustomPropertyTypeLayout("VoxelWorldGeneratorPicker"     , FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVoxelWorldGeneratorPickerCustomization     ::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("VoxelMaterialCollectionElement", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVoxelMaterialCollectionElementCustomization::MakeInstance));

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	void UnregisterClassLayout()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule != nullptr)
		{
			PropertyModule->UnregisterCustomClassLayout("VoxelWorld");
			PropertyModule->UnregisterCustomClassLayout("VoxelMaterialCollection");
			
			PropertyModule->UnregisterCustomPropertyTypeLayout("VoxelWorldGeneratorPicker");

			PropertyModule->NotifyCustomizationModuleChanged();
		}
	}

	/** Registers asset tool actions. */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_VoxelWorldSaveObject(    VoxelAssetCategoryBit)));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_VoxelMaterialCollection( VoxelAssetCategoryBit)));
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

			for (auto& Action : RegisteredAssetTypeActions)
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
