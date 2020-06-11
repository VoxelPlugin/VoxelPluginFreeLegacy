// Copyright 2020 Phyronnaz

#include "VoxelEditorModule.h"
#include "VoxelMinimal.h"

#include "Interfaces/IPluginManager.h"
#include "IPlacementModeModule.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "EditorModeRegistry.h"
#include "EditorSupportDelegates.h"
#include "LevelEditor.h"
#include "EngineUtils.h"
#include "MessageLogModule.h"
#include "EditorReimportHandler.h"
#include "Framework/Commands/Commands.h"

#include "VoxelGraphGenerator.h"
#include "VoxelWorld.h"
#include "VoxelTexture.h"
#include "VoxelPlaceableItems/VoxelDisableEditsBox.h"
#include "VoxelPlaceableItems/VoxelAssetActor.h"
#include "VoxelMessages.h"
#include "VoxelMessagesEditor.h"

#include "AssetTools/AssetTypeActions_VoxelDataAsset.h"
#include "AssetTools/AssetTypeActions_VoxelHeightmapAsset.h"
#include "AssetTools/AssetTypeActions_VoxelGraphWorldGenerator.h"
#include "AssetTools/AssetTypeActions_VoxelGraphOutputsConfig.h"
#include "AssetTools/AssetTypeActions_VoxelSpawnerConfig.h"
#include "AssetTools/AssetTypeActions_VoxelSpawners.h"
#include "AssetTools/AssetTypeActions_VoxelGraphMacro.h"
#include "AssetTools/AssetTypeActions_VoxelWorldSaveObject.h"
#include "AssetTools/AssetTypeActions_VoxelMaterialCollection.h"

#include "Thumbnails/VoxelGraphGeneratorThumbnailRenderer.h"
#include "Thumbnails/VoxelSpawnersThumbnailRenderer.h"
#include "Thumbnails/VoxelDataAssetThumbnailRenderer.h"
#include "Thumbnails/VoxelHeightmapAssetThumbnailRenderer.h"

#include "EdMode/VoxelEdMode.h"

#include "ActorFactoryVoxelWorld.h"
#include "ActorFactoryVoxelPlaceableItems.h"
#include "ActorFactoryVoxelMeshImporter.h"

#include "Details/VoxelWorldDetails.h"
#include "Details/VoxelLandscapeImporterDetails.h"
#include "Details/VoxelMeshImporterDetails.h"
#include "Details/VoxelAssetActorDetails.h"
#include "Details/VoxelWorldGeneratorPickerCustomization.h"
#include "Details/RangeAnalysisDebuggerDetails.h"
#include "Details/VoxelPaintMaterialCustomization.h"
#include "Details/VoxelMeshSpawnerBaseDetails.h"
#include "Details/VoxelBasicSpawnerScaleSettingsCustomization.h"
#include "Details/VoxelSpawnerOutputNameCustomization.h"
#include "Details/VoxelSpawnerDensityCustomization.h"
#include "Details/VoxelSpawnerConfigSpawnerCustomization.h"
#include "Details/VoxelGraphOutputCustomization.h"
#include "Details/VoxelInt32IntervalCustomization.h"
#include "Details/VoxelBoolVectorCustomization.h"

#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelImporters/VoxelLandscapeImporter.h"
#include "VoxelComponents/VoxelInvokerComponent.h"

#include "Factories/VoxelWorldSaveObjectFactory.h"
#include "VoxelEditorDetailsUtilities.h"
#include "VoxelWorldEditorControls.h"

const FVector2D Icon14x14(14.0f, 14.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);
const FVector2D Icon512x512(512.0f, 512.0f);

class FVoxelEditorCommands : public TCommands<FVoxelEditorCommands>
{
public:
	FVoxelEditorCommands()
		: TCommands<FVoxelEditorCommands>
		(
		"VoxelEditor", // Context name for icons
		VOXEL_LOCTEXT("Voxel Editor"), // Localized context name for displaying
		NAME_None, // Parent
		"VoxelStyle" // Icon Style Set
		)
	{
	}
	
	TSharedPtr<FUICommandInfo> RefreshVoxelWorlds;

#define LOCTEXT_NAMESPACE "Voxel"
	virtual void RegisterCommands() override
	{
		UI_COMMAND(
			RefreshVoxelWorlds, 
			"Retoggle", 
			"Retoggle the voxel worlds", 
			EUserInterfaceActionType::Button, 
			FInputChord(EModifierKey::Control, EKeys::F5));
	}
#undef LOCTEXT_NAMESPACE
};

static void RefreshVoxelWorlds_Execute(UObject* MatchingGenerator = nullptr)
{
	FViewport* Viewport = GEditor->GetActiveViewport();
	if (Viewport)
	{
		FViewportClient* Client = Viewport->GetClient();
		if (Client)
		{
			UWorld* World = Client->GetWorld();
			if (World && (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview))
			{
				for (TActorIterator<AVoxelWorld> It(World); It; ++It)
				{
					if (It->IsCreated() && (!MatchingGenerator || It->WorldGenerator.GetObject() == MatchingGenerator))
					{
						It->Toggle();
						It->Toggle();
					}
				}
				for (TActorIterator<AVoxelAssetActor> It(World); It; ++It)
				{
					It->UpdatePreview();
				}
			}
		}
	}
}

static void BindEditorDelegates(IVoxelEditorDelegatesInterface* Interface, UObject* Object)
{
	check(Interface && Object);
	
	if (!FEditorDelegates::PreSaveWorld.IsBoundToObject(Object))
	{
		FEditorDelegates::PreSaveWorld.AddWeakLambda(Object, [=](uint32 SaveFlags, UWorld* World) { Interface->OnPreSaveWorld(SaveFlags, World); });
	}
	if (!FEditorDelegates::PreBeginPIE.IsBoundToObject(Object))
	{
		FEditorDelegates::PreBeginPIE.AddWeakLambda(Object, [=](bool bIsSimulating) { Interface->OnPreBeginPIE(bIsSimulating); });
	}
	if (!FEditorDelegates::EndPIE.IsBoundToObject(Object))
	{
		FEditorDelegates::EndPIE.AddWeakLambda(Object, [=](bool bIsSimulating) { Interface->OnEndPIE(bIsSimulating); });
	}
	if (!FEditorDelegates::OnApplyObjectToActor.IsBoundToObject(Object))
	{
		FEditorDelegates::OnApplyObjectToActor.AddWeakLambda(Object, [=](UObject* InObject, AActor* InActor) { Interface->OnApplyObjectToActor(InObject, InActor); });
	}
	if (!FEditorSupportDelegates::PrepareToCleanseEditorObject.IsBoundToObject(Object))
	{
		FEditorSupportDelegates::PrepareToCleanseEditorObject.AddWeakLambda(Object, [=](UObject* InObject) { Interface->OnPrepareToCleanseEditorObject(InObject); });
	}
	if (!FCoreDelegates::OnPreExit.IsBoundToObject(Object))
	{
		FCoreDelegates::OnPreExit.AddWeakLambda(Object, [=]() { Interface->OnPreExit(); });
	}
}

class FVoxelWorldEditor : public IVoxelWorldEditor
{
public:
	FVoxelWorldEditor() = default;

	virtual UVoxelWorldSaveObject* CreateSaveObject() override
	{
		return Cast<UVoxelWorldSaveObject>(FVoxelEditorUtilities::CreateAssetWithDialog(UVoxelWorldSaveObject::StaticClass(), NewObject<UVoxelWorldSaveObjectFactory>()));
	}

	virtual UClass* GetVoxelWorldEditorClass() override
	{
		return AVoxelWorldEditorControls::StaticClass();
	}

};

/**
 * Implements the VoxelEditor module.
 */
class FVoxelEditorModule : public IVoxelEditorModule
{
public:
	virtual void StartupModule() override
	{
		// Voxel World Editor
		if (!IVoxelWorldEditor::GetVoxelWorldEditor())
		{
			IVoxelWorldEditor::SetVoxelWorldEditor(MakeShared<FVoxelWorldEditor>());
		}
		
		// Clear texture cache on reimport
		FReimportManager::Instance()->OnPostReimport().AddLambda([](UObject*, bool) { FVoxelTextureUtilities::ClearCache(); });
		
		// Global commands
		FVoxelEditorCommands::Register();

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetGlobalLevelEditorActions()->MapAction(
			FVoxelEditorCommands::Get().RefreshVoxelWorlds,
			FExecuteAction::CreateStatic(&RefreshVoxelWorlds_Execute, (UObject*)nullptr),
			FCanExecuteAction());

		IVoxelEditorDelegatesInterface::BindEditorDelegatesDelegate.AddStatic(&BindEditorDelegates);

		// Blueprint errors
		FVoxelMessages::LogMessageDelegate.AddStatic(&FVoxelMessagesEditor::LogMessage);
		FVoxelMessages::ShowNotificationDelegate.AddStatic(&FVoxelMessagesEditor::ShowNotification);

		FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
		FMessageLogInitializationOptions InitOptions;
		InitOptions.bShowFilters = true;
		InitOptions.bShowPages = false;
		InitOptions.bAllowClear = true;
		MessageLogModule.RegisterLogListing("Voxel", VOXEL_LOCTEXT("Voxel"), InitOptions);

		// Voxel asset category
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		VoxelAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory("Voxel", VOXEL_LOCTEXT("Voxel"));

		RegisterPlacementModeExtensions();
		RegisterCustomClassLayouts();
		RegisterAssetTools();
		
		// Thumbnails
		auto& ThumbnailManager = UThumbnailManager::Get();
		ThumbnailManager.RegisterCustomRenderer(UVoxelGraphGenerator  ::StaticClass(), UVoxelGraphGeneratorThumbnailRenderer  ::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelDataAsset       ::StaticClass(), UVoxelDataAssetThumbnailRenderer       ::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelHeightmapAsset  ::StaticClass(), UVoxelHeightmapAssetThumbnailRenderer  ::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelMeshSpawner     ::StaticClass(), UVoxelMeshSpawnerThumbnailRenderer     ::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelMeshSpawnerGroup::StaticClass(), UVoxelMeshSpawnerGroupThumbnailRenderer::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelAssetSpawner    ::StaticClass(), UVoxelAssetSpawnerThumbnailRenderer    ::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UVoxelSpawnerGroup    ::StaticClass(), UVoxelSpawnerGroupThumbnailRenderer    ::StaticClass());

		// Icons
		{
			FString ContentDir = IPluginManager::Get().FindPlugin(VOXEL_PLUGIN_NAME)->GetContentDir() + "/";

			StyleSet = MakeShareable(new FSlateStyleSet("VoxelStyle"));
			StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
			StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

			// VoxelWorld
			StyleSet->Set("ClassThumbnail.VoxelWorld"                        , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorld"                             , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/World_16x.png"), Icon16x16));
																		     
			// Voxel Material Collection								     
			StyleSet->Set("ClassThumbnail.VoxelMaterialCollectionBase"       , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/AssetIcons/PaperTileMap_64x.png")), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelMaterialCollectionBase"            , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/AssetIcons/PaperTileMap_16x.png")), Icon16x16));

			// Importers
			StyleSet->Set("ClassThumbnail.VoxelLandscapeImporter"            , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Import_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelLandscapeImporter"                 , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Import_16x.png"), Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelMeshImporter"                 , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Import_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelMeshImporter"                      , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Import_16x.png"), Icon16x16));
			
			// Spawners
			StyleSet->Set("ClassThumbnail.VoxelSpawnerConfig"                , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerConfig_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelSpawnerConfig"                     , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerConfig_16x.png"), Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelSpawner"                      , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Spawner_64x.png")  , Icon64x64));
			StyleSet->Set("ClassIcon.VoxelSpawner"                           , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Spawner_16x.png")  , Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelSpawnerGroup"                 , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerGroup_64x.png")  , Icon64x64));
			StyleSet->Set("ClassIcon.VoxelSpawnerGroup"                      , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerGroup_16x.png")  , Icon16x16));
			StyleSet->Set("ClassThumbnail.VoxelMeshSpawnerGroup"             , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerGroup_64x.png")  , Icon64x64));
			StyleSet->Set("ClassIcon.VoxelMeshSpawnerGroup"                  , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/SpawnerGroup_16x.png")  , Icon16x16));
																		     
			// Voxel Graph												     
			StyleSet->Set("ClassThumbnail.VoxelGraphGenerator"               , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelGraph_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelGraphGenerator"                    , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelGraph_16x.png"), Icon16x16));
																		     
			// Data Asset												     
			StyleSet->Set("ClassThumbnail.VoxelDataAsset"                    , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/DataAsset_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelDataAsset"                         , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/DataAsset_16x.png"), Icon16x16));
																		     
			// Landscape asset											     
			StyleSet->Set("ClassThumbnail.VoxelLandscapeAsset"               , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Landscape_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelLandscapeAsset"                    , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/Landscape_16x.png"), Icon16x16));	
																		     
			// Data Asset Editor										     
			StyleSet->Set("VoxelDataAssetEditor.InvertDataAsset"             , new FSlateImageBrush(StyleSet->RootToContentDir(TEXT("Icons/icon_rotateb_40x.png")), Icon40x40));
																		     
			// Voxel Editor Tools										     
			StyleSet->Set("VoxelTools.Tab"                                   , new FSlateImageBrush(ContentDir + TEXT("Icons/UIIcons/mode_40.png"), Icon40x40));
			StyleSet->Set("VoxelTools.Tab.Small"                             , new FSlateImageBrush(ContentDir + TEXT("Icons/UIIcons/mode_40.png"), Icon16x16));
																		     
			// World generator											     
			StyleSet->Set("ClassThumbnail.VoxelWorldGenerator"               , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorldGenerator"                    , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/WorldGenerator_16x.png"), Icon16x16));
																		     
			// Voxel World Object Save									     
			StyleSet->Set("ClassThumbnail.VoxelWorldSaveObject"              , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelWorldSaveObject_64x.png"), Icon64x64));
			StyleSet->Set("ClassIcon.VoxelWorldSaveObject"                   , new FSlateImageBrush(ContentDir + TEXT("Icons/AssetIcons/VoxelWorldSaveObject_16x.png"), Icon16x16));
			
			FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
		}

		// Voxel Editor Tools
		FEditorModeRegistry::Get().RegisterMode<FEdModeVoxel>(FEdModeVoxel::EM_Voxel, VOXEL_LOCTEXT("Voxels"), FSlateIcon("VoxelStyle", "VoxelTools.Tab", "VoxelTools.Tab.Small"), true);
	}

	virtual void ShutdownModule() override
	{
		FEditorModeRegistry::Get().UnregisterMode(FEdModeVoxel::EM_Voxel);

		if (UObjectInitialized())
		{
			auto& ThumbnailManager = UThumbnailManager::Get();
			ThumbnailManager.UnregisterCustomRenderer(UVoxelGraphGenerator::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelDataAsset::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelHeightmapAsset::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelMeshSpawner::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelMeshSpawnerGroup::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelAssetSpawner::StaticClass());
			ThumbnailManager.UnregisterCustomRenderer(UVoxelSpawnerGroup::StaticClass());
		}

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

		RegisterPlacementModeExtension<AVoxelWorld          >(PlacementModeModule, GetMutableDefault<UActorFactoryVoxelWorld          >());
		RegisterPlacementModeExtension<AVoxelDisableEditsBox>(PlacementModeModule, GetMutableDefault<UActorFactoryVoxelDisableEditsBox>());
		RegisterPlacementModeExtension<AVoxelAssetActor     >(PlacementModeModule, GetMutableDefault<UActorFactoryVoxelAssetActor     >());
		RegisterPlacementModeExtension<AVoxelMeshImporter   >(PlacementModeModule, GetMutableDefault<UActorFactoryVoxelMeshImporter   >());

		RegisterPlacementModeExtension<AVoxelLODVolume>(PlacementModeModule);
		RegisterPlacementModeExtension<AVoxelLandscapeImporter>(PlacementModeModule);

		PlacementModeModule.RegenerateItemsForCategory(FBuiltInPlacementCategories::AllClasses());
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
		PropertyModule.RegisterCustomClassLayout(Name, FOnGetDetailCustomizationInstance::CreateLambda([]() { return MakeShared<T>(); }));
		RegisteredCustomClassLayouts.Add(Name);
	}
	template<typename T>
	void RegisterCustomPropertyLayout(FPropertyEditorModule& PropertyModule, FName Name)
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(Name, FOnGetPropertyTypeCustomizationInstance::CreateLambda([]() { return MakeShared<T>(); }));
		RegisteredCustomPropertyLayouts.Add(Name);
	}

	void RegisterCustomClassLayouts()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		RegisterCustomClassLayout<FVoxelWorldDetails                    >(PropertyModule, "VoxelWorld");
		RegisterCustomClassLayout<FVoxelLandscapeImporterDetails        >(PropertyModule, "VoxelLandscapeImporter");
		RegisterCustomClassLayout<FVoxelMeshImporterDetails             >(PropertyModule, "VoxelMeshImporter");
		RegisterCustomClassLayout<FVoxelAssetActorDetails               >(PropertyModule, "VoxelAssetActor");
		RegisterCustomClassLayout<FRangeAnalysisDebuggerDetails         >(PropertyModule, "VoxelNode_RangeAnalysisDebuggerFloat");
		RegisterCustomClassLayout<FVoxelMeshSpawnerBaseDetails          >(PropertyModule, "VoxelMeshSpawnerBase");

		// TODO type safe
		RegisterCustomPropertyLayout<FVoxelWorldGeneratorPickerCustomization                   >(PropertyModule, "VoxelWorldGeneratorPicker");
		RegisterCustomPropertyLayout<FVoxelWorldGeneratorPickerCustomization                   >(PropertyModule, "VoxelTransformableWorldGeneratorPicker");
		RegisterCustomPropertyLayout<FVoxelPaintMaterialCustomization                          >(PropertyModule, "VoxelPaintMaterial");
		RegisterCustomPropertyLayout<FVoxelPaintMaterial_MaterialCollectionChannelCustomization>(PropertyModule, "VoxelPaintMaterial_MaterialCollectionChannel");
		RegisterCustomPropertyLayout<FVoxelBoolVectorCustomization                             >(PropertyModule, "VoxelBoolVector");
		RegisterCustomPropertyLayout<FVoxelBasicSpawnerScaleSettingsCustomization              >(PropertyModule, "VoxelBasicSpawnerScaleSettings");
		RegisterCustomPropertyLayout<FVoxelSpawnerOutputNameCustomization                      >(PropertyModule, "VoxelSpawnerOutputName");
		RegisterCustomPropertyLayout<FVoxelSpawnerDensityCustomization                         >(PropertyModule, "VoxelSpawnerDensity");
		RegisterCustomPropertyLayout<FVoxelSpawnerConfigSpawnerCustomization                   >(PropertyModule, "VoxelSpawnerConfigSpawner");
		RegisterCustomPropertyLayout<FVoxelGraphOutputCustomization                            >(PropertyModule, "VoxelGraphOutput");
		RegisterCustomPropertyLayout<FVoxelInt32IntervalCustomization                          >(PropertyModule, "VoxelInt32Interval");

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
	void RegisterAssetTypeAction()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		auto Action = MakeShared<T>(VoxelAssetCategoryBit);
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	void RegisterAssetTools()
	{
		RegisterAssetTypeAction<FAssetTypeActions_VoxelWorldSaveObject>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelBasicMaterialCollection>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelInstancedMaterialCollectionTemplates>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelInstancedMaterialCollection>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelInstancedMaterialCollectionInstance>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelDataAsset>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelSpawnerConfig>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelAssetSpawner>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelMeshSpawner>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelMeshSpawnerGroup>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelSpawnerGroup>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelHeightmapAsset>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelGraphWorldGenerator>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelGraphMacro>();
		RegisterAssetTypeAction<FAssetTypeActions_VoxelGraphOutputsConfig>();
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

	virtual void RefreshVoxelWorlds(UObject* MatchingGenerator) override
	{
		RefreshVoxelWorlds_Execute(MatchingGenerator);
	}

	virtual EAssetTypeCategories::Type GetVoxelAssetTypeCategory() const override
	{
		return VoxelAssetCategoryBit;
	}

private:
	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TArray<FName> RegisteredCustomClassLayouts;
	TArray<FName> RegisteredCustomPropertyLayouts;

	EAssetTypeCategories::Type VoxelAssetCategoryBit = EAssetTypeCategories::None;
	FPlacementCategoryInfo PlacementCategoryInfo = FPlacementCategoryInfo(VOXEL_LOCTEXT("Voxel"), "Voxel", TEXT("PMVoxel"), 25);
	TSharedPtr<FSlateStyleSet> StyleSet;
};

IMPLEMENT_MODULE(FVoxelEditorModule, VoxelEditor);