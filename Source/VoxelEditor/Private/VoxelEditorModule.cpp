// Copyright 2019 Phyronnaz

#include "VoxelEditorModule.h"

#include "Interfaces/IPluginManager.h"
#include "IPlacementModeModule.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "EditorModeRegistry.h"

#include "VoxelWorld.h"

#include "AssetTools/AssetTypeActions_VoxelWorldSaveObject.h"
#include "AssetTools/AssetTypeActions_VoxelMaterialCollection.h"



#include "Factories/VoxelWorldFactory.h"

#include "Details/VoxelWorldDetails.h"
#include "Details/VoxelWorldGeneratorPickerCustomization.h"
#include "Details/VoxelMaterialCollectionDetails.h"
#include "Details/VoxelMaterialCollectionHelpers.h"


#define LOCTEXT_NAMESPACE "Voxel"

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
	virtual void StartupModule() override
	{
		// Voxel asset category
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		VoxelAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory("Voxel", LOCTEXT("VoxelAssetCategory", "Voxel"));

		RegisterPlacementModeExtensions();
		RegisterCustomClassLayouts();
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

private:
	template<typename T>
	void RegisterPlacementModeExtension(IPlacementModeModule& PlacementModeModule, UActorFactory* Factory = nullptr)
	{
		PlacementModeModule.RegisterPlaceableItem(PlacementCategoryInfo.UniqueHandle, MakeShared<FPlaceableItem>(Factory, FAssetData(T::StaticClass())));
	}
	void RegisterPlacementModeExtensions()
	{
		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.RegisterPlacementCategory(PlacementCategoryInfo);

		RegisterPlacementModeExtension<AVoxelWorld       >(PlacementModeModule, GetMutableDefault<UVoxelWorldFactory       >());
	}
	void UnregisterPlacementModeExtensions()
	{
		if (IPlacementModeModule::IsAvailable())
		{
			IPlacementModeModule::Get().UnregisterPlacementCategory(PlacementCategoryInfo.UniqueHandle);
		}
	}

private:
	template<typename T>
	void RegisterCustomClassLayout(FPropertyEditorModule& PropertyModule, FName Name)
	{
		PropertyModule.RegisterCustomClassLayout(Name, FOnGetDetailCustomizationInstance::CreateStatic(&T::MakeInstance));
		RegisteredCustomClassLayouts.Add(Name);
	}
	template<typename T>
	void RegisterCustomPropertyLayout(FPropertyEditorModule& PropertyModule, FName Name)
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(Name, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&T::MakeInstance));
		RegisteredCustomPropertyLayouts.Add(Name);
	}

	void RegisterCustomClassLayouts()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		RegisterCustomClassLayout<FVoxelWorldDetails                    >(PropertyModule, "VoxelWorld");
		RegisterCustomClassLayout<FVoxelMaterialCollectionDetails       >(PropertyModule, "VoxelMaterialCollection");

		RegisterCustomPropertyLayout<FVoxelWorldGeneratorPickerCustomization      >(PropertyModule, "VoxelWorldGeneratorPicker");
		RegisterCustomPropertyLayout<FVoxelMaterialCollectionElementCustomization >(PropertyModule, "VoxelMaterialCollectionElement");

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	void UnregisterClassLayout()
	{
		if (auto* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (auto& Name : RegisteredCustomClassLayouts)
			{
				PropertyModule->UnregisterCustomClassLayout(Name);
			}
			for (auto& Name : RegisteredCustomPropertyLayouts)
			{
				PropertyModule->UnregisterCustomPropertyTypeLayout(Name);
			}
			PropertyModule->NotifyCustomizationModuleChanged();
		}
	}
	
private:
	template<typename T>
	void RegisterAssetTypeAction(IAssetTools& AssetTools)
	{
		auto Action = MakeShared<T>(VoxelAssetCategoryBit);
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction<FAssetTypeActions_VoxelWorldSaveObject    >(AssetTools);
		RegisterAssetTypeAction<FAssetTypeActions_VoxelMaterialCollection >(AssetTools);
	}

	void UnregisterAssetTools()
	{
		if (auto* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();
			for (auto& Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}
	
public:

	virtual bool GenerateSingleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) override
	{
		return FVoxelMaterialCollectionHelpers::GenerateSingleMaterials(Collection, OutError);
	}

	virtual bool GenerateDoubleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) override
	{
		return FVoxelMaterialCollectionHelpers::GenerateDoubleMaterials(Collection, OutError);
	}

	virtual bool GenerateTripleMaterials(UVoxelMaterialCollection* Collection, FString& OutError) override
	{
		return FVoxelMaterialCollectionHelpers::GenerateTripleMaterials(Collection, OutError);
	}

private:
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TArray<FName> RegisteredCustomClassLayouts;
	TArray<FName> RegisteredCustomPropertyLayouts;

	EAssetTypeCategories::Type VoxelAssetCategoryBit;
	FPlacementCategoryInfo PlacementCategoryInfo = FPlacementCategoryInfo(LOCTEXT("VoxelCategoryName", "Voxel"), "Voxel", TEXT("PMVoxel"), 25);
	TSharedPtr<FSlateStyleSet> StyleSet;
};

IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);

#undef LOCTEXT_NAMESPACE