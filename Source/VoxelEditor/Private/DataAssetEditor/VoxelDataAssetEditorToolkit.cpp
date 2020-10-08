// Copyright 2020 Phyronnaz

#include "VoxelDataAssetEditorToolkit.h"
#include "VoxelEditorToolsPanel.h"
#include "DataAssetEditor/VoxelDataAssetEditorCommands.h"
#include "DataAssetEditor/VoxelDataAssetEditorManager.h"
#include "DataAssetEditor/SVoxelDataAssetEditorViewport.h"
#include "Details/VoxelWorldDetails.h"

#include "VoxelWorld.h"
#include "VoxelFeedbackContext.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.h"
#include "VoxelTools/VoxelAssetTools.h"
#include "VoxelUtilities/VoxelConfigUtilities.h"

#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "AdvancedPreviewSceneModule.h"
#include "AdvancedPreviewScene.h"
#include "PreviewScene.h"
#include "EngineUtils.h"
#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Misc/MessageDialog.h"

const FName FVoxelDataAssetEditorToolkit::EditToolsTabId(TEXT("VoxelDataAssetEditor_EditTools"));
const FName FVoxelDataAssetEditorToolkit::PreviewSettingsTabId(TEXT("VoxelDataAssetEditor_PreviewSettings"));
const FName FVoxelDataAssetEditorToolkit::DetailsTabId(TEXT("VoxelDataAssetEditor_Details"));
const FName FVoxelDataAssetEditorToolkit::AdvancedPreviewSettingsTabId(TEXT("VoxelDataAssetEditor_AdvancedPreviewSettings"));
const FName FVoxelDataAssetEditorToolkit::PreviewTabId(TEXT("VoxelDataAssetEditor_Preview"));

class FVoxelAdvancedPreviewScene : public FAdvancedPreviewScene
{
public:
	FVoxelAdvancedPreviewScene()
		: FAdvancedPreviewScene(ConstructionValues())
	{
		if (SkyComponent)
		{
			SkyComponent->SetWorldScale3D(FVector(1000000));
		}
	}
};

FVoxelDataAssetEditorToolkit::FVoxelDataAssetEditorToolkit()
{
	PreviewScene = MakeShared<FVoxelAdvancedPreviewScene>();
	PreviewScene->SetFloorVisibility(false);
	PreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);

	UWorld* PreviewWorld = PreviewScene->GetWorld();
	for (FActorIterator It(PreviewWorld); It; ++It)
	{
		It->DispatchBeginPlay();
	}
	PreviewWorld->bBegunPlay = true;
}

FVoxelDataAssetEditorToolkit::~FVoxelDataAssetEditorToolkit()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(VOXEL_LOCTEXT("Voxel Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(EditToolsTabId, FOnSpawnTab::CreateSP(this, &FVoxelDataAssetEditorToolkit::SpawnTab_EditTools))
		.SetDisplayName(VOXEL_LOCTEXT("Edit Tools"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PreviewSettingsTabId, FOnSpawnTab::CreateSP(this, &FVoxelDataAssetEditorToolkit::SpawnTab_PreviewSettings))
		.SetDisplayName(VOXEL_LOCTEXT("Preview Settings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FVoxelDataAssetEditorToolkit::SpawnTab_Details))
		.SetDisplayName(VOXEL_LOCTEXT("Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(AdvancedPreviewSettingsTabId, FOnSpawnTab::CreateSP(this, &FVoxelDataAssetEditorToolkit::SpawnTab_AdvancedPreviewSettings))
		.SetDisplayName(VOXEL_LOCTEXT("Advanced Preview Settings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FVoxelDataAssetEditorToolkit::SpawnTab_Preview))
		.SetDisplayName(VOXEL_LOCTEXT("Preview"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FVoxelDataAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(EditToolsTabId);
	InTabManager->UnregisterTabSpawner(PreviewSettingsTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(AdvancedPreviewSettingsTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);
}

void FVoxelDataAssetEditorToolkit::InitVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit)
{
	DataAsset = CastChecked<UVoxelDataAsset>(ObjectToEdit);
	
	// Support undo/redo
	DataAsset->SetFlags(RF_Transactional);

	Manager = MakeUnique<FVoxelDataAssetEditorManager>(DataAsset, *PreviewScene);

	ToolsPanel = MakeShared<FVoxelEditorToolsPanel>();
	ToolsPanel->Init();
	// To have a nice screenshot
	ToolsPanel->ClearTool();
	
	FVoxelDataAssetEditorCommands::Register();

	BindCommands();
	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_VoxelDataAssetEditor_Layout_v3")
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.1f)
			->SetHideTabWell( true )
			->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		->Split
		(
			FTabManager::NewSplitter() ->SetOrientation(Orient_Horizontal) ->SetSizeCoefficient(0.9f)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation(Orient_Vertical) ->SetSizeCoefficient(0.2f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab( EditToolsTabId, ETabState::OpenedTab )
					->AddTab( PreviewSettingsTabId, ETabState::OpenedTab)
					->AddTab( AdvancedPreviewSettingsTabId, ETabState::ClosedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab( DetailsTabId, ETabState::OpenedTab)
				)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation( Orient_Vertical )
				->SetSizeCoefficient(0.80f)
				->Split
				(
					FTabManager::NewStack() 
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell( true )
					->AddTab( PreviewTabId, ETabState::OpenedTab )
				)
				
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("VoxelDataAssetEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit, false);

	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetEditorToolkit::CreateInternalWidgets()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	{
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		Args.NotifyHook = nullptr;
		Args.bShowOptions = false;
		Args.bShowActorLabel = false;

		PreviewSettings = PropertyModule.CreateDetailView(Args);
		PreviewSettings->RegisterInstancedCustomPropertyLayout(
			AVoxelWorld::StaticClass(),
			FOnGetDetailCustomizationInstance::CreateLambda([]() { return MakeShared<FVoxelWorldDetails>(true); }));
		PreviewSettings->SetObject(&Manager->GetVoxelWorld());
		PreviewSettings->OnFinishedChangingProperties().AddLambda([=](const FPropertyChangedEvent& Event)
		{
			if (Event.ChangeType != EPropertyChangeType::Interactive)
			{
				if (DataAsset->bUseSettingsAsDefault)
				{
					FVoxelConfigUtilities::SaveConfig(&Manager->GetVoxelWorld(), "VoxelDataAssetEditor.DefaultVoxelWorld");
				}
			}
		});
	}

	{
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		Args.NotifyHook = this;
		Args.bShowOptions = false;
		Args.bShowActorLabel = false;
		Details = PropertyModule.CreateDetailView(Args);
		Details->SetObject(DataAsset);
	}
	
	FAdvancedPreviewSceneModule& AdvancedPreviewSceneModule = FModuleManager::LoadModuleChecked<FAdvancedPreviewSceneModule>("AdvancedPreviewScene");
	AdvancedPreviewSettingsWidget = AdvancedPreviewSceneModule.CreateAdvancedPreviewSceneSettingsWidget(PreviewScene.ToSharedRef());

	Preview = SNew(SVoxelDataAssetEditorViewport).Editor(this);
}

void FVoxelDataAssetEditorToolkit::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &FVoxelDataAssetEditorToolkit::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}

void FVoxelDataAssetEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	auto& Commands = FVoxelDataAssetEditorCommands::Get();
	
	ToolbarBuilder.BeginSection("Toolbar");
	ToolbarBuilder.AddToolBarButton(Commands.InvertDataAsset);
	ToolbarBuilder.EndSection();
}

void FVoxelDataAssetEditorToolkit::BindCommands()
{
	auto& Commands = FVoxelDataAssetEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.InvertDataAsset,
		FExecuteAction::CreateSP(this, &FVoxelDataAssetEditorToolkit::InvertDataAsset));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetEditorToolkit::SaveAsset_Execute()
{
	FVoxelScopedSlowTask Progress(2, VOXEL_LOCTEXT("Saving asset"));
 	Progress.MakeDialog(false, true);

	Progress.EnterProgressFrame();
	Manager->Save(true);
	
	Progress.EnterProgressFrame();
	FAssetEditorToolkit::SaveAsset_Execute();
}

bool FVoxelDataAssetEditorToolkit::OnRequestClose()
{
	if (Manager->IsDirty())
	{
		const auto Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			EAppReturnType::Cancel,
			FText::Format(VOXEL_LOCTEXT("Voxel Data Asset {0}: \nSave your changes?"),
				FText::FromString(DataAsset->GetName())));
		if (Result == EAppReturnType::Yes)
		{
			SaveAsset_Execute();
			return true;
		}
		else if (Result == EAppReturnType::No)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DataAsset);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetEditorToolkit::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive ||
		!ensure(PropertyChangedEvent.GetNumObjectsBeingEdited() == 1) ||
		!ensure(PropertyChangedEvent.MemberProperty) ||
		!ensure(PropertyChangedEvent.GetObjectBeingEdited(0) == DataAsset))
	{
		return;
	}

	const auto SaveAsset = [&]()
	{
		if (Manager->IsDirty())
		{
			FVoxelScopedSlowTask Progress(1, VOXEL_LOCTEXT("Saving asset"));
			Progress.MakeDialog(false, true);
			Progress.EnterProgressFrame();
			Manager->Save(false);
		}
	};

	const FName Name = PropertyChangedEvent.MemberProperty->GetFName();
	if (Name == GET_MEMBER_NAME_STATIC(UVoxelDataAsset, bSubtractiveAsset))
	{
		if (Manager->IsDirty())
		{
			const bool bNewSubtractiveAsset = DataAsset->bSubtractiveAsset;
			const bool bOldSubtractiveAsset = !bNewSubtractiveAsset;
			DataAsset->bSubtractiveAsset = bOldSubtractiveAsset;

			SaveAsset();

			DataAsset->bSubtractiveAsset = bNewSubtractiveAsset;
		}
		Manager->RecreateWorld();
	}
	else if (Name == GET_MEMBER_NAME_STATIC(UVoxelDataAsset, PositionOffset))
	{
		// Save position offset, as it will be changed by Save
		const FIntVector PositionOffset = DataAsset->PositionOffset;

		SaveAsset();

		DataAsset->PositionOffset = PositionOffset;
		Manager->RecreateWorld();
	}
	else if (Name == GET_MEMBER_NAME_STATIC(UVoxelDataAsset, Tolerance))
	{
		SaveAsset();
		Manager->RecreateWorld();
	}
	else if (Name == GET_MEMBER_NAME_STATIC(UVoxelDataAsset, bUseSettingsAsDefault))
	{
		// Nothing to do
	}
	else
	{
		ensure(false);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FAdvancedPreviewScene& FVoxelDataAssetEditorToolkit::GetPreviewScene() const
{
	return *PreviewScene;
}

AVoxelWorld& FVoxelDataAssetEditorToolkit::GetVoxelWorld() const
{
	return Manager->GetVoxelWorld();
}

UVoxelDataAsset& FVoxelDataAssetEditorToolkit::GetDataAsset() const
{
	return *DataAsset;
}

FVoxelEditorToolsPanel& FVoxelDataAssetEditorToolkit::GetPanel() const
{
	return *ToolsPanel;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SDockTab> FVoxelDataAssetEditorToolkit::SpawnTab_EditTools(const FSpawnTabArgs& Args)
{	
	check(Args.GetTabId() == EditToolsTabId);

	auto Tab =
		 SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(VOXEL_LOCTEXT("Edit Tools"))
		[
			ToolsPanel->GetWidget()
		];
	return Tab;
}

TSharedRef<SDockTab> FVoxelDataAssetEditorToolkit::SpawnTab_PreviewSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewSettingsTabId);

	auto Tab =
		 SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(VOXEL_LOCTEXT("Preview Settings"))
		[
			PreviewSettings.ToSharedRef()
		];
	return Tab;
}

TSharedRef<SDockTab> FVoxelDataAssetEditorToolkit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	auto Tab =
		 SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(VOXEL_LOCTEXT("Details"))
		[
			Details.ToSharedRef()
		];
	return Tab;
}

TSharedRef<SDockTab> FVoxelDataAssetEditorToolkit::SpawnTab_AdvancedPreviewSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == AdvancedPreviewSettingsTabId);

	auto Tab =
		 SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(VOXEL_LOCTEXT("Advanced Preview Settings"))
		[
			AdvancedPreviewSettingsWidget.ToSharedRef()
		];
	return Tab;
}

TSharedRef<SDockTab> FVoxelDataAssetEditorToolkit::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewTabId);

	auto Tab =
		 SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Viewports"))
		.Label(VOXEL_LOCTEXT("Preview"))
		[
			Preview.ToSharedRef()
		];
	return Tab;
}

void FVoxelDataAssetEditorToolkit::InvertDataAsset()
{
	if (Manager->IsDirty())
	{
		FVoxelScopedSlowTask Progress(1, VOXEL_LOCTEXT("Saving asset"));
		Progress.MakeDialog(false, true);
		Progress.EnterProgressFrame();
		Manager->Save(false);
	}
	
	const auto NewData = MakeVoxelShared<FVoxelDataAssetData>();
	UVoxelAssetTools::InvertDataAssetImpl(*DataAsset->GetData(), *NewData);
	DataAsset->SetData(NewData);
	
	Manager->RecreateWorld();
}