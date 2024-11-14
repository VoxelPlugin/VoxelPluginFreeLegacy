// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelGraphEditorToolkit.h"
#include "VoxelGraphEditorUtilities.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphShortcuts.h"
#include "VoxelGraphPreviewSettings.h"
#include "VoxelGraphSchema.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelNodes/VoxelLocalVariables.h"
#include "VoxelGraphEditorCommands.h"
#include "VoxelGraphNodes/VoxelGraphNode_Root.h"
#include "VoxelGraphNodes/VoxelGraphNode.h"
#include "VoxelGraphNodes/SVoxelGraphNode.h"
#include "VoxelGraphNodes/VoxelGraphNode_Knot.h"
#include "VoxelMessages.h"

#include "VoxelDebugGraphUtils.h"
#include "VoxelEditorModule.h"

#include "SVoxelPalette.h"
#include "Preview/SVoxelGraphPreview.h"
#include "Preview/SVoxelGraphPreviewViewport.h"
#include "Preview/VoxelGraphPreview.h"

#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "ScopedTransaction.h"
#include "Misc/MessageDialog.h"
#include "HAL/PlatformApplicationMisc.h"

#include "Editor.h"
#include "EditorStyleSet.h"
#include "GraphEditorActions.h"
#include "GraphEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/GenericCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SScaleBox.h"

#include "EdGraph/EdGraph.h"
#include "EdGraphUtilities.h"
#include "AdvancedPreviewScene.h"

#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"

const FName FVoxelGraphEditorToolkit::GraphCanvasTabId(TEXT("VoxelGraphEditor_GraphCanvas"));
const FName FVoxelGraphEditorToolkit::DebugGraphCanvasTabId(TEXT("VoxelGraphEditor_DebugGraphCanvas"));
const FName FVoxelGraphEditorToolkit::PropertiesTabId(TEXT("VoxelGraphEditor_Properties"));
const FName FVoxelGraphEditorToolkit::ShortcutsTabId(TEXT("VoxelGraphEditor_Shortcuts"));
const FName FVoxelGraphEditorToolkit::PreviewSettingsTabId(TEXT("VoxelGraphEditor_PreviewSettings"));
const FName FVoxelGraphEditorToolkit::PaletteTabId(TEXT("VoxelGraphEditor_Palette"));
const FName FVoxelGraphEditorToolkit::PreviewTabId(TEXT("VoxelGraphEditor_Preview"));
const FName FVoxelGraphEditorToolkit::PreviewViewportTabId(TEXT("VoxelGraphEditor_PreviewViewport"));
const FName FVoxelGraphEditorToolkit::MessagesTabId(TEXT("VoxelGraphEditor_Messages"));

FVoxelGraphEditorToolkit::FVoxelGraphEditorToolkit()
{

}

FVoxelGraphEditorToolkit::~FVoxelGraphEditorToolkit()
{
	GEditor->UnregisterForUndo(this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(VOXEL_LOCTEXT("Voxel Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_GraphCanvas))
		.SetDisplayName(VOXEL_LOCTEXT("Main Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(DebugGraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_DebugGraphCanvas))
		.SetDisplayName(VOXEL_LOCTEXT("Debug Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(PropertiesTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_Properties))
		.SetDisplayName(VOXEL_LOCTEXT("Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(ShortcutsTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_Shortcuts))
		.SetDisplayName(VOXEL_LOCTEXT("Shortcuts"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PreviewSettingsTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_PreviewSettings))
		.SetDisplayName(VOXEL_LOCTEXT("Preview Settings"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(PaletteTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_Palette))
		.SetDisplayName(VOXEL_LOCTEXT("Palette"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.Palette"));

	InTabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_Preview))
		.SetDisplayName(VOXEL_LOCTEXT("Preview"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(PreviewViewportTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_PreviewViewport))
		.SetDisplayName(VOXEL_LOCTEXT("3D Preview"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(MessagesTabId, FOnSpawnTab::CreateSP(this, &FVoxelGraphEditorToolkit::SpawnTab_Messages))
		.SetDisplayName(VOXEL_LOCTEXT("Messages"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "MessageLog.TabIcon"));
}

void FVoxelGraphEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphCanvasTabId);
	InTabManager->UnregisterTabSpawner(DebugGraphCanvasTabId);
	InTabManager->UnregisterTabSpawner(PropertiesTabId);
	InTabManager->UnregisterTabSpawner(ShortcutsTabId);
	InTabManager->UnregisterTabSpawner(PreviewSettingsTabId);
	InTabManager->UnregisterTabSpawner(PaletteTabId);
	InTabManager->UnregisterTabSpawner(PreviewTabId);
	InTabManager->UnregisterTabSpawner(PreviewViewportTabId);
	InTabManager->UnregisterTabSpawner(MessagesTabId);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::InitVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit)
{
	FVoxelMessages::Info("You can view and edit Voxel Graphs, but running them requires Voxel Plugin Pro");
	
	Generator = CastChecked<UVoxelGraphGenerator>(ObjectToEdit);

	if (!ensureAlways(Generator->VoxelGraph && Generator->VoxelDebugGraph))
	{
		FAssetEditorToolkit::InitAssetEditor(
			Mode,
			InitToolkitHost,
			TEXT("VoxelGraphEditorApp"),
			FTabManager::NewLayout("Standalone_VoxelGraphEditor_Crash")->AddArea(FTabManager::NewPrimaryArea()),
			false,
			false,
			ObjectToEdit,
			false);
		return;
	}

	// Support undo/redo
	Generator->SetFlags(RF_Transactional);

	GEditor->RegisterForUndo(this);

	FGraphEditorCommands::Register();
	FVoxelGraphEditorCommands::Register();

	BindGraphCommands();

	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_VoxelGraphEditor_Layout_v9")
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewSplitter() ->SetOrientation(Orient_Horizontal) ->SetSizeCoefficient(0.9f)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation(Orient_Vertical) ->SetSizeCoefficient(0.2f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab( PaletteTabId, ETabState::ClosedTab )
					->AddTab( PreviewSettingsTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab( ShortcutsTabId, ETabState::OpenedTab )
					->AddTab( PropertiesTabId, ETabState::OpenedTab )
				)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation( Orient_Vertical )
				->SetSizeCoefficient(0.7f)
				->Split
				(
					FTabManager::NewStack() 
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell( true )
					->AddTab( GraphCanvasTabId, ETabState::OpenedTab )
					->AddTab( DebugGraphCanvasTabId, ETabState::ClosedTab )
				)
				->Split
				(
					FTabManager::NewStack() 
					->SetSizeCoefficient(0.2f)
					->AddTab( MessagesTabId, ETabState::OpenedTab )
				)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation(Orient_Vertical) 
				->SetSizeCoefficient(0.3f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell( true )
					->AddTab( PreviewTabId, ETabState::OpenedTab )
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell( true )
					->AddTab( PreviewViewportTabId, ETabState::OpenedTab )
				)
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, TEXT("VoxelGraphEditorApp"), StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit, false);

	ExtendToolbar();
	ExtendMenu();
	RegenerateMenusAndToolbars();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::CreateInternalWidgets()
{
	VoxelGraphEditor = CreateGraphEditorWidget(false);
	VoxelDebugGraphEditor = CreateGraphEditorWidget(true);

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.NotifyHook = this;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	VoxelProperties = PropertyModule.CreateDetailView(Args);
	VoxelProperties->SetObject(Generator);

	ShortcutsProperties = PropertyModule.CreateDetailView(Args);
	ShortcutsProperties->SetObject(GetMutableDefault<UVoxelGraphShortcuts>());

	if (!Generator->PreviewSettings)
	{
		Generator->PreviewSettings = NewObject<UVoxelGraphPreviewSettings>(Generator);
		Generator->PreviewSettings->Graph = Generator;
	}

	// Needed for undo/redo
	Generator->PreviewSettings->SetFlags(RF_Transactional);

	PreviewSettings = PropertyModule.CreateDetailView(Args);
	PreviewSettings->SetObject(Generator->PreviewSettings);
	
	// Must be created before PreviewViewport
	PreviewScene = MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues()));

	Palette = SNew(SVoxelPalette);
	Preview = SNew(SVoxelGraphPreview).PreviewSettings(Generator->PreviewSettings);
	PreviewViewport = SNew(SVoxelGraphPreviewViewport).VoxelGraphEditorToolkit(SharedThis(this));
	
	PreviewHandler = MakeShared<FVoxelGraphPreview>(Generator, Preview, PreviewViewport, PreviewScene);

	Preview->SetTexture(Generator->GetPreviewTexture());

	// Messages panel
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions LogOptions;
	LogOptions.bShowPages = false;
	LogOptions.bShowFilters = true;
	LogOptions.bAllowClear = false;
	LogOptions.MaxPageCount = 1;

	MessagesListing = MessageLogModule.CreateLogListing("VoxelGraphEditorErrors", LogOptions);
	MessagesWidget = MessageLogModule.CreateLogListingWidget(MessagesListing.ToSharedRef());
}

void FVoxelGraphEditorToolkit::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Toolbar");
	
	auto& Commands = FVoxelGraphEditorCommands::Get();
	ToolbarBuilder.AddToolBarButton(Commands.CompileToCpp);
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddToolBarButton(Commands.ToggleAutomaticPreview);
	ToolbarBuilder.AddToolBarButton(Commands.UpdatePreview);
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddToolBarButton(Commands.ClearNodesMessages);
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddToolBarButton(Commands.ShowAxisDependencies);
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddToolBarButton(Commands.ShowStats);
	ToolbarBuilder.AddToolBarButton(Commands.ShowValues);

	ToolbarBuilder.EndSection();
}

void FVoxelGraphEditorToolkit::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateRaw(this, &FVoxelGraphEditorToolkit::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}

void FVoxelGraphEditorToolkit::FillVoxelMenu(FMenuBuilder& MenuBuilder)
{
	auto& Commands = FVoxelGraphEditorCommands::Get();

	MenuBuilder.AddMenuEntry(Commands.RecreateNodes);
}

void FVoxelGraphEditorToolkit::AddEditorMenus(FMenuBarBuilder& MenuBarBuilder)
{
	MenuBarBuilder.AddPullDownMenu(
		VOXEL_LOCTEXT("Voxel"),
		VOXEL_LOCTEXT("Open the Voxel menu"),
		FNewMenuDelegate::CreateRaw(this, &FVoxelGraphEditorToolkit::FillVoxelMenu),
		"Voxel");
}

void FVoxelGraphEditorToolkit::ExtendMenu()
{
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

	MenuExtender->AddMenuBarExtension(
		"Edit",
		EExtensionHook::After,
		GetToolkitCommands(),
		FMenuBarExtensionDelegate::CreateRaw(this, &FVoxelGraphEditorToolkit::AddEditorMenus));

	AddMenuExtender(MenuExtender);
}

void FVoxelGraphEditorToolkit::BindGraphCommands()
{
	auto& Commands = FVoxelGraphEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.CompileToCpp,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CompileToCpp));

	ToolkitCommands->MapAction(
		Commands.RecreateNodes,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::RecreateNodes));
	
	ToolkitCommands->MapAction(
		Commands.ToggleAutomaticPreview,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::ToggleAutomaticPreview),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &FVoxelGraphEditorToolkit::IsToggleAutomaticPreviewChecked));

	ToolkitCommands->MapAction(
		Commands.UpdatePreview,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::UpdatePreview, EVoxelGraphPreviewFlags::UpdateAll | EVoxelGraphPreviewFlags::ManualPreview));
	
	ToolkitCommands->MapAction(
		Commands.UpdateVoxelWorlds,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::UpdateVoxelWorlds));

	ToolkitCommands->MapAction(
		Commands.ClearNodesMessages,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::ClearNodesMessages));

	ToolkitCommands->MapAction(
		Commands.ShowAxisDependencies,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::ShowAxisDependencies));

	ToolkitCommands->MapAction(
		Commands.ShowStats,
		FExecuteAction::CreateWeakLambda(Generator.Get(), [this]()
		{
			Generator->PreviewSettings->bShowStats = !Generator->PreviewSettings->bShowStats;
			UpdatePreview(EVoxelGraphPreviewFlags::UpdateTextures);
		}),
		FCanExecuteAction(),
		FIsActionChecked::CreateWeakLambda(Generator.Get(), [this]() { return Generator->PreviewSettings->bShowStats; }));

	ToolkitCommands->MapAction(
		Commands.ShowValues,
		FExecuteAction::CreateWeakLambda(Generator.Get(), [this]()
		{
			Generator->PreviewSettings->bShowValues = !Generator->PreviewSettings->bShowValues;
			UpdatePreview(EVoxelGraphPreviewFlags::UpdateTextures);
		}),
		FCanExecuteAction(),
		FIsActionChecked::CreateWeakLambda(Generator.Get(), [this]() { return Generator->PreviewSettings->bShowValues; }));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Undo,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::UndoGraphAction));

	ToolkitCommands->MapAction(
		FGenericCommands::Get().Redo,
		FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::RedoGraphAction));
}

TSharedRef<SGraphEditor> FVoxelGraphEditorToolkit::CreateGraphEditorWidget(bool bDebug)
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList);

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().AddInput,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::AddInput),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanAddInput));

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().DeleteInput,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::DeleteInput),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanDeleteInput));

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().TogglePinPreview,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnTogglePinPreview));

		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().SplitPin,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnSplitPin));
		GraphEditorCommands->MapAction(FVoxelGraphEditorCommands::Get().CombinePin,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnCombinePin));

		// Graph Editor Commands
		GraphEditorCommands->MapAction(FGraphEditorCommands::Get().CreateComment,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnCreateComment)
		);

		// Editing commands
		GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::SelectAllNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanSelectAllNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::DeleteSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanDeleteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CopySelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanCopyNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CutSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanCutNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::PasteNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanPasteNodes)
		);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::DuplicateNodes),
			FCanExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::CanDuplicateNodes)
		);

		GraphEditorCommands->MapAction( FVoxelGraphEditorCommands::Get().SelectLocalVariableDeclaration,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnSelectLocalVariableDeclaration)
			);

		GraphEditorCommands->MapAction( FVoxelGraphEditorCommands::Get().SelectLocalVariableUsages,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnSelectLocalVariableUsages)
			);

		GraphEditorCommands->MapAction( FVoxelGraphEditorCommands::Get().ConvertRerouteToVariables,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnConvertRerouteToVariables)
			);

		GraphEditorCommands->MapAction( FVoxelGraphEditorCommands::Get().ConvertVariablesToReroute,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::OnConvertVariablesToReroute)
			);

		GraphEditorCommands->MapAction( FVoxelGraphEditorCommands::Get().ReconstructNode,
			FExecuteAction::CreateSP(this, &FVoxelGraphEditorToolkit::ReconstructNode)
			);

		// Alignment Commands
		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesTop,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignTop )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesMiddle,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignMiddle )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesBottom,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignBottom )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesLeft,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignLeft )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesCenter,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignCenter )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().AlignNodesRight,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnAlignRight )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().StraightenConnections,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnStraightenConnections )
			);

		// Distribution Commands
		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().DistributeNodesHorizontally,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnDistributeNodesH )
			);

		GraphEditorCommands->MapAction( FGraphEditorCommands::Get().DistributeNodesVertically,
			FExecuteAction::CreateSP( this, &FVoxelGraphEditorToolkit::OnDistributeNodesV )
			);
	}

	if (bDebug)
	{
		FGraphAppearanceInfo AppearanceInfo;
		AppearanceInfo.CornerText = VOXEL_LOCTEXT("VOXEL DEBUG");

		return SNew(SGraphEditor)
			.IsEditable(true)
			.Appearance(AppearanceInfo)
			.GraphToEdit(Generator->VoxelDebugGraph)
			.AutoExpandActionMenu(false)
			.ShowGraphStateOverlay(false);
	}
	else
	{
		FGraphAppearanceInfo AppearanceInfo;
		AppearanceInfo.CornerText = VOXEL_LOCTEXT("VOXEL");

		SGraphEditor::FGraphEditorEvents InEvents;
		InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FVoxelGraphEditorToolkit::OnSelectedNodesChanged);
		InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FVoxelGraphEditorToolkit::OnNodeTitleCommitted);
		InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FVoxelGraphEditorToolkit::OnNodeDoubleClicked);
		InEvents.OnSpawnNodeByShortcut = SGraphEditor::FOnSpawnNodeByShortcut::CreateSP(this, &FVoxelGraphEditorToolkit::OnSpawnGraphNodeByShortcut);

		return SNew(SGraphEditor)
			.AdditionalCommands(GraphEditorCommands)
			.IsEditable(true)
			.Appearance(AppearanceInfo)
			.GraphToEdit(Generator->VoxelGraph)
			.GraphEvents(InEvents)
			.AutoExpandActionMenu(false)
			.ShowGraphStateOverlay(false);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool FVoxelGraphEditorToolkit::GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding)
{
	return VoxelGraphEditor->GetBoundsForSelectedNodes(Rect, Padding);
}

int32 FVoxelGraphEditorToolkit::GetNumberOfSelectedNodes() const
{
	return VoxelGraphEditor->GetSelectedNodes().Num();
}

FGraphPanelSelectionSet FVoxelGraphEditorToolkit::GetSelectedNodes() const
{
	return VoxelGraphEditor->GetSelectedNodes();
}

void FVoxelGraphEditorToolkit::SelectNodesAndZoomToFit(const TArray<UEdGraphNode*>& Nodes)
{
	if (Nodes.Num() > 0)
	{
		VoxelGraphEditor->ClearSelectionSet();
		for (auto& Node : Nodes)
		{
			VoxelGraphEditor->SetNodeSelection(Node, true);
		}
		VoxelGraphEditor->ZoomToFit(true);
	}
}

void FVoxelGraphEditorToolkit::RefreshNodesMessages()
{
	for (auto Node : Generator->VoxelGraph->Nodes)
	{
		if (Node->IsA<UVoxelGraphNode>() && !Node->IsA<UVoxelGraphNode_Knot>())
		{
			TSharedPtr<SGraphNode> Widget = Node->DEPRECATED_NodeWidget.Pin();
			if (Widget.IsValid())
			{
				static_cast<SVoxelGraphNode*>(Widget.Get())->RefreshErrorInfo();
			}
		}
	}
}

void FVoxelGraphEditorToolkit::TriggerUpdatePreview(EVoxelGraphPreviewFlags Flags)
{
	if (Generator->bAutomaticPreview || EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::ManualPreview))
	{
		bUpdatePreviewOnNextTick = true;
		NextPreviewFlags |= Flags;
	}
}

FAdvancedPreviewScene* FVoxelGraphEditorToolkit::GetPreviewScene() const
{
	return PreviewScene.Get();
}

void FVoxelGraphEditorToolkit::DebugNodes(const TSet<FVoxelCompilationNode*>& Nodes)
{
}

inline EMessageSeverity::Type VoxelMessageTypeToMessageSeverity(EVoxelGraphNodeMessageType Type)
{
	switch (Type)
	{
	default: ensure(false);
	case EVoxelGraphNodeMessageType::Info:
		return EMessageSeverity::Info;
	case EVoxelGraphNodeMessageType::Warning:
		return EMessageSeverity::Warning;
	case EVoxelGraphNodeMessageType::Error:
		return EMessageSeverity::Error;
	}
}

void FVoxelGraphEditorToolkit::AddMessages(const TArray<FVoxelGraphMessage>& Messages)
{
	CurrentMessages.Append(Messages);

	TArray<TSharedRef<FTokenizedMessage>> ListingMessages;
	for (auto& Message : Messages)
	{
		TSharedRef<FTokenizedMessage> ListingMessage = FTokenizedMessage::Create(VoxelMessageTypeToMessageSeverity(Message.Type));
		if (Message.Node.IsValid())
		{
			ListingMessage->AddToken(FActionToken::Create(
				Message.Node->GetTitle(),
				Message.Node->GetTitle(),
				FOnActionTokenExecuted::CreateSP(
					this,
					&FVoxelGraphEditorToolkit::SelectNodeAndZoomToFit,
					Message.Node)
			));
		}
		ListingMessage->AddToken(FTextToken::Create(FText::FromString(Message.Message)));
		ListingMessages.Add(ListingMessage);
	}
	MessagesListing->AddMessages(ListingMessages, false);
}

void FVoxelGraphEditorToolkit::ClearMessages(bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear)
{
	MessagesListing->ClearMessages();
	if (bClearAll)
	{
		CurrentMessages.Reset();
	}
	else
	{
		TArray<FVoxelGraphMessage> Copy = CurrentMessages;
		Copy.RemoveAll([&](auto& Message) { return Message.Type == MessagesToClear; });
		CurrentMessages.Reset();

		AddMessages(Copy);
	}
}

void FVoxelGraphEditorToolkit::SaveAsset_Execute()
{
	if (Generator->bCompileToCppOnSave)
	{
	}

	// Make sure to save AFTER compile to cpp to avoid dirtying it again
	FAssetEditorToolkit::SaveAsset_Execute();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Generator);
	if (PreviewHandler.IsValid())
	{
		PreviewHandler->AddReferencedObjects(Collector);
	}
}

void FVoxelGraphEditorToolkit::PostUndo(bool bSuccess)
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->ClearSelectionSet();
		VoxelGraphEditor->NotifyGraphChanged();
	}
	TriggerUpdatePreview(EVoxelGraphPreviewFlags::UpdateAll);
}

void FVoxelGraphEditorToolkit::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	if (VoxelGraphEditor.IsValid() && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		VoxelGraphEditor->NotifyGraphChanged();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::Tick(float DeltaTime)
{
	if (bUpdatePreviewOnNextTick)
	{
		UpdatePreview(NextPreviewFlags);
		bUpdatePreviewOnNextTick = false;
		NextPreviewFlags = EVoxelGraphPreviewFlags::None;
	}
}

TStatId FVoxelGraphEditorToolkit::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FVoxelGraphEditorToolkit, STATGROUP_Tickables);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == GraphCanvasTabId);

	auto Tab = SNew(SDockTab)
		      .Label(VOXEL_LOCTEXT("Main Graph"));
	
	GraphTab = Tab;
	GraphTab->SetContent(VoxelGraphEditor.ToSharedRef());
	
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_DebugGraphCanvas(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DebugGraphCanvasTabId);

	 auto Tab = SNew(SDockTab)
		        .Label(VOXEL_LOCTEXT("Debug Graph"));

	DebugGraphTab = Tab;
	DebugGraphTab->SetContent(VoxelDebugGraphEditor.ToSharedRef());

	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_Properties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PropertiesTabId);
	
	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Details"))
		[
			VoxelProperties.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_Shortcuts(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ShortcutsTabId);
	
	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Shortcuts"))
		[
			ShortcutsProperties.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_PreviewSettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewSettingsTabId);
	
	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Preview Settings"))
		[
			PreviewSettings.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Details"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_Palette(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PaletteTabId);
	
	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Palette"))
		[
			Palette.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("Kismet.Tabs.Palette"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewTabId);
	
	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Preview"))
		[
			// Do the scaling here to make math easier
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			[
				Preview.ToSharedRef()
			]
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Viewports"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_PreviewViewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewViewportTabId);

	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("3D Preview"))
		[
			PreviewViewport.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("LevelEditor.Tabs.Viewports"));
	return Tab;
}

TSharedRef<SDockTab> FVoxelGraphEditorToolkit::SpawnTab_Messages(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == MessagesTabId);

	auto Tab =
		 SNew(SDockTab)
		.Label(VOXEL_LOCTEXT("Messages"))
		[
			MessagesWidget.ToSharedRef()
		];
	Tab->SetTabIcon(FAppStyle::GetBrush("MessageLog.TabIcon"));
	return Tab;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	TArray<UObject*> Selection;

	if (NewSelection.Num())
	{
		for (auto* Object : NewSelection)
		{
			if (Cast<UVoxelGraphNode_Root>(Object) || Cast<UVoxelGraphMacroInputOutputNode>(Object))
			{
				Selection.Add(Generator);
			}
			else if (UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(Object))
			{
				Selection.Add(GraphNode->VoxelNode);
			}
			else
			{
				Selection.Add(Object);
			}
		}
	}
	else
	{
		Selection.Add(Generator);
	}
	
	VoxelProperties->SetObjects(Selection);
}

void FVoxelGraphEditorToolkit::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged)
{
	if (NodeBeingChanged)
	{
		const FScopedTransaction Transaction(VOXEL_LOCTEXT("Rename Node"));
		NodeBeingChanged->Modify();
		NodeBeingChanged->OnRenameNode(NewText.ToString());
	}
}

void FVoxelGraphEditorToolkit::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	if (Node->CanJumpToDefinition())
	{
		Node->JumpToDefinition();
	}
}

FReply FVoxelGraphEditorToolkit::OnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2D& InPosition)
{
	auto* Ptr = GetDefault<UVoxelGraphShortcuts>()->Shortcuts.FindByPredicate([&](auto& Key) { return Key.IsSameAs(InChord); });
	UClass* ClassToSpawn = Ptr ? Ptr->Class : nullptr;
	if (ClassToSpawn)
	{
		FVoxelGraphSchemaAction_NewNode Action(FText(), FText(), FText(), 0);
		Action.VoxelNodeClass = ClassToSpawn;
		Action.PerformAction(Generator->VoxelGraph, nullptr, InPosition);
	}

	return FReply::Handled();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::AddInput()
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	// Iterator used but should only contain one node
	for (auto* SelectedNode : SelectedNodes)
	{
		if (auto* Node = Cast<UVoxelGraphNode>(SelectedNode))
		{
			Node->AddInputPin();
			break;
		}
	}
}

bool FVoxelGraphEditorToolkit::CanAddInput() const
{
	return GetSelectedNodes().Num() == 1;
}

void FVoxelGraphEditorToolkit::DeleteInput()
{
	UEdGraphPin* SelectedPin = VoxelGraphEditor->GetGraphPinForMenu();
	UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(SelectedPin->GetOwningNode());

	if (SelectedNode && SelectedNode == SelectedPin->GetOwningNode())
	{
		SelectedNode->RemoveInputPin(SelectedPin);
	}
}

bool FVoxelGraphEditorToolkit::CanDeleteInput() const
{
	return true;
}

void FVoxelGraphEditorToolkit::OnCreateComment()
{
	FVoxelGraphSchemaAction_NewComment CommentAction;
	CommentAction.PerformAction(Generator->VoxelGraph, NULL, VoxelGraphEditor->GetPasteLocation());
}

void FVoxelGraphEditorToolkit::OnTogglePinPreview()
{	
	UEdGraphPin* SelectedPin = VoxelGraphEditor->GetGraphPinForMenu();
	UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(SelectedPin->GetOwningNode());
	UVoxelGraphNode* GraphNodeToPreview = Cast<UVoxelGraphNode>(SelectedNode);
	if (GraphNodeToPreview && GraphNodeToPreview->VoxelNode)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const bool bIsPreviewing = SelectedPin->bIsDiffing;

		if (Generator->PreviewedPin.Get())
		{
			ensure(!bIsPreviewing || SelectedPin == Generator->PreviewedPin.Get());
			ensure(Generator->PreviewedPin.Get()->bIsDiffing);
			Generator->PreviewedPin.Get()->bIsDiffing = false;
			Generator->PreviewedPin.SetPin(nullptr);
		}

		ensure(!SelectedPin->bIsDiffing);
		if (!bIsPreviewing)
		{
			SelectedPin->bIsDiffing = true;
			Generator->PreviewedPin.SetPin(SelectedPin);
		}
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		VoxelGraphEditor->NotifyGraphChanged();
	}
	UpdatePreview(EVoxelGraphPreviewFlags::UpdateAll | EVoxelGraphPreviewFlags::ManualPreview);
}

void FVoxelGraphEditorToolkit::OnSplitPin()
{
	UEdGraphPin* SelectedPin = VoxelGraphEditor->GetGraphPinForMenu();
	UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(SelectedPin->GetOwningNode());
	SelectedNode->TrySplitPin(*SelectedPin, false);
}

void FVoxelGraphEditorToolkit::OnCombinePin()
{
	UEdGraphPin* SelectedPin = VoxelGraphEditor->GetGraphPinForMenu();
	UVoxelGraphNode* SelectedNode = Cast<UVoxelGraphNode>(SelectedPin->GetOwningNode());
	SelectedNode->TryCombinePin(*SelectedPin, false);
}

void FVoxelGraphEditorToolkit::SelectAllNodes()
{
	VoxelGraphEditor->SelectAllNodes();
}

void FVoxelGraphEditorToolkit::DeleteSelectedNodes()
{
	const FScopedTransaction Transaction(VOXEL_LOCTEXT("Delete Selected Voxel Node"));

	VoxelGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	VoxelGraphEditor->ClearSelectionSet();

	for (auto* Object : SelectedNodes)
	{
		UEdGraphNode* Node = CastChecked<UEdGraphNode>(Object);

		if (Node->CanUserDeleteNode())
		{
			if (UVoxelGraphNode* VoxelGraphNode = Cast<UVoxelGraphNode>(Node))
			{
				UVoxelNode* VoxelNode = VoxelGraphNode->VoxelNode;
				if (VoxelNode)
				{
					VoxelNode->Modify();
					VoxelNode->MarkAsGarbage();
				}

				auto* PreviewedPin = Generator->PreviewedPin.Get();
				if (PreviewedPin && PreviewedPin->GetOwningNode() == VoxelGraphNode)
				{
					// Clear previewed pin if we delete the owning node
					Generator->PreviewedPin = {};
					// Clear since we're not previewing it anymore
					PRAGMA_DISABLE_DEPRECATION_WARNINGS
					PreviewedPin->bIsDiffing = false;
					PRAGMA_ENABLE_DEPRECATION_WARNINGS
				}

				FBlueprintEditorUtils::RemoveNode(NULL, VoxelGraphNode, true);

				// Make sure Voxel is updated to match graph
				Generator->CompileVoxelNodesFromGraphNodes();

				// Remove this node from the list of all VoxelNodes
				Generator->AllNodes.Remove(VoxelNode);
				Generator->Modify();
			}
			else
			{
				FBlueprintEditorUtils::RemoveNode(NULL, Node, true);
			}
		}
	}
}

bool FVoxelGraphEditorToolkit::CanDeleteNodes() const
{
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();

	if (SelectedNodes.Num() == 1)
	{
		for (auto* Node : SelectedNodes)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(Node);
			if (GraphNode && !GraphNode->CanUserDeleteNode())
			{
				return false;
			}
		}
	}

	return SelectedNodes.Num() > 0;
}

void FVoxelGraphEditorToolkit::DeleteSelectedDuplicatableNodes()
{
	// Cache off the old selection
	const FGraphPanelSelectionSet OldSelectedNodes = GetSelectedNodes();

	// Clear the selection and only select the nodes that can be duplicated
	FGraphPanelSelectionSet RemainingNodes;
	VoxelGraphEditor->ClearSelectionSet();

	for (auto* SelectedNode : OldSelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(SelectedNode);
		if (Node && Node->CanDuplicateNode())
		{
			VoxelGraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// Delete the duplicable nodes
	DeleteSelectedNodes();

	// Reselect whatever's left from the original selection after the deletion
	VoxelGraphEditor->ClearSelectionSet();

	for (auto* RemainingNode : RemainingNodes)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(RemainingNode))
		{
			VoxelGraphEditor->SetNodeSelection(Node, true);
		}
	}
}

void FVoxelGraphEditorToolkit::CutSelectedNodes()
{
	CopySelectedNodes();
	// Cut should only delete nodes that can be duplicated
	DeleteSelectedDuplicatableNodes();
}

bool FVoxelGraphEditorToolkit::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FVoxelGraphEditorToolkit::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	FGraphPanelSelectionSet SelectedNodes;
	{
		FGraphPanelSelectionSet AllSelectedNodes = GetSelectedNodes();
		for (auto* SelectedNode : AllSelectedNodes)
		{
			auto* Node = Cast<UEdGraphNode>(SelectedNode);
			if (Node && Node->CanDuplicateNode())
			{
				SelectedNodes.Add(Node);
			}
		}
	}

	FString ExportedText;

	for (auto It = SelectedNodes.CreateIterator(); It; ++It)
	{
		CastChecked<UEdGraphNode>(*It)->PrepareForCopying();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	// Make sure the voxel graph remains the owner of the copied nodes
	for (auto It = SelectedNodes.CreateIterator(); It; ++It)
	{
		if (auto* Node = Cast<UVoxelGraphNode>(*It))
		{
			Node->PostCopyNode();
		}
	}
}

bool FVoxelGraphEditorToolkit::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	for (auto* SelectedNode : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(SelectedNode);
		if (Node && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;
}

void FVoxelGraphEditorToolkit::PasteNodes()
{
	PasteNodesHere(VoxelGraphEditor->GetPasteLocation());
}

void FVoxelGraphEditorToolkit::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(VOXEL_LOCTEXT("Paste Voxel Node"));
	Generator->VoxelGraph->Modify();
	Generator->Modify();

	// Clear the selection set (newly pasted stuff will be selected)
	VoxelGraphEditor->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(Generator->VoxelGraph, TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (auto* Node : PastedNodes)
	{
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if (PastedNodes.Num() > 0)
	{
		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	TArray<UVoxelNode*> PastedVoxelNodes;
	for (auto* Node : PastedNodes)
	{
		if (UVoxelGraphNode* VoxelGraphNode = Cast<UVoxelGraphNode>(Node))
		{
			if (UVoxelNode* VoxelNode = VoxelGraphNode->VoxelNode)
			{
				PastedVoxelNodes.Add(VoxelNode);
				Generator->AllNodes.Add(VoxelNode);
				VoxelNode->Graph = Generator;
			}
		}

		// Select the newly pasted stuff
		VoxelGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	// Force new pasted VoxelNodes to have same connections as graph nodes
	Generator->CompileVoxelNodesFromGraphNodes();

	// Post copy for local variables
	for (auto* Node : PastedVoxelNodes)
	{
		Node->PostCopyNode(PastedVoxelNodes);
	}

	// Update UI
	VoxelGraphEditor->NotifyGraphChanged();

	Generator->PostEditChange();
	Generator->MarkPackageDirty();
}

bool FVoxelGraphEditorToolkit::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(Generator->VoxelGraph, ClipboardContent);
}

void FVoxelGraphEditorToolkit::DuplicateNodes()
{
	// Copy and paste current selection
	CopySelectedNodes();
	PasteNodes();
}

bool FVoxelGraphEditorToolkit::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FVoxelGraphEditorToolkit::OnAlignTop()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignTop();
	}
}

void FVoxelGraphEditorToolkit::OnAlignMiddle()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignMiddle();
	}
}

void FVoxelGraphEditorToolkit::OnAlignBottom()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignBottom();
	}
}

void FVoxelGraphEditorToolkit::OnAlignLeft()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignLeft();
	}
}

void FVoxelGraphEditorToolkit::OnAlignCenter()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignCenter();
	}
}

void FVoxelGraphEditorToolkit::OnAlignRight()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnAlignRight();
	}
}

void FVoxelGraphEditorToolkit::OnStraightenConnections()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnStraightenConnections();
	}
}

void FVoxelGraphEditorToolkit::OnDistributeNodesH()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnDistributeNodesH();
	}
}

void FVoxelGraphEditorToolkit::OnDistributeNodesV()
{
	if (VoxelGraphEditor.IsValid())
	{
		VoxelGraphEditor->OnDistributeNodesV();
	}
}

void FVoxelGraphEditorToolkit::OnSelectLocalVariableDeclaration()
{
	const FGraphPanelSelectionSet SelectedNodes = VoxelGraphEditor->GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		VoxelGraphEditor->ClearSelectionSet();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(*NodeIt);
			if (GraphNode)
			{
				UVoxelNode* CurrentSelectedNoe = GraphNode->VoxelNode;
				UVoxelLocalVariableUsage* Usage = Cast<UVoxelLocalVariableUsage>(CurrentSelectedNoe);
				if (Usage && Usage->Declaration)
				{
					UEdGraphNode* DeclarationGraphNode = Usage->Declaration->GraphNode;
					if (DeclarationGraphNode)
					{
						VoxelGraphEditor->SetNodeSelection(DeclarationGraphNode, true);
					}
				}
			}
		}
		VoxelGraphEditor->ZoomToFit(true);
	}
}

void FVoxelGraphEditorToolkit::OnSelectLocalVariableUsages()
{
	const FGraphPanelSelectionSet SelectedNodes = VoxelGraphEditor->GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		bool bZoom = false;
		VoxelGraphEditor->ClearSelectionSet();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(*NodeIt);
			if (GraphNode)
			{
				UVoxelNode* CurrentSelectedNode = GraphNode->VoxelNode;
				UVoxelLocalVariableDeclaration* Declaration = Cast<UVoxelLocalVariableDeclaration>(CurrentSelectedNode);
				for (UVoxelNode* Node : Generator->AllNodes)
				{
					auto* Usage = Cast<UVoxelLocalVariableUsage>(Node);
					if (Usage && Usage->Declaration == Declaration)
					{
						UEdGraphNode* UsageGraphNode = Usage->GraphNode;
						if (UsageGraphNode)
						{
							bZoom = true;
							VoxelGraphEditor->SetNodeSelection(UsageGraphNode, true);
						}
					}
				}
			}
		}
		if (bZoom)
		{
			VoxelGraphEditor->ZoomToFit(true);
		}
	}
}

void FVoxelGraphEditorToolkit::OnConvertRerouteToVariables()
{
	const FGraphPanelSelectionSet SelectedNodes = VoxelGraphEditor->GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		VoxelGraphEditor->ClearSelectionSet();
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UVoxelGraphNode_Knot* GraphNode = Cast<UVoxelGraphNode_Knot>(*NodeIt);
			if (GraphNode)
			{
				UEdGraph* Graph = GraphNode->GetGraph();
				const FScopedTransaction Transaction(VOXEL_LOCTEXT("Convert reroute to local variables"));
				Graph->Modify();
				
				const TArray<UEdGraphPin*>& InputPins = GraphNode->GetInputPin()->LinkedTo;
				TArray<UEdGraphPin*> OutputPins = GraphNode->GetOutputPin()->LinkedTo;
				OutputPins.Sort([](UEdGraphPin& A, UEdGraphPin& B) { return A.GetOwningNode()->NodePosY < B.GetOwningNode()->NodePosY; });

				TArray<UVoxelLocalVariableUsage*> Usages;
				int UsageIndex = -OutputPins.Num() / 2;
				for (auto* OutputPin : OutputPins)
				{
					auto* Usage = Generator->ConstructNewNode<UVoxelLocalVariableUsage>(FVector2D(GraphNode->NodePosX + 50, GraphNode->NodePosY + 50 * UsageIndex));
					Usages.Add(Usage);
					UsageIndex++;
				}

				// Spawn declaration AFTER usages so that it gets renamed
				auto* Declaration = Generator->ConstructNewNode<UVoxelLocalVariableDeclaration>(FVector2D(GraphNode->NodePosX - 50, GraphNode->NodePosY));
				Declaration->SetCategory(FVoxelPinCategory::FromString(GraphNode->GetInputPin()->PinType.PinCategory));
				Declaration->GraphNode->ReconstructNode();

				check(Declaration->GraphNode->Pins.Num() == 1);
				UEdGraphPin* DeclarationInputPin = Declaration->GraphNode->Pins[0];
				check(DeclarationInputPin->Direction == EEdGraphPinDirection::EGPD_Input)
				for (auto* InputPin : InputPins)
				{
					InputPin->MakeLinkTo(DeclarationInputPin);
				}

				for (int32 Index = 0; Index < OutputPins.Num() ; Index++)
				{
					auto* Usage = Usages[Index];
					Usage->Declaration = Declaration;
					Usage->DeclarationGuid = Declaration->VariableGuid;
					Usage->GraphNode->ReconstructNode();
					Usage->GraphNode->GetAllPins()[0]->MakeLinkTo(OutputPins[Index]); // usage node has a single pin
				}

				GraphNode->DestroyNode();
			}
		}
	}
}

void FVoxelGraphEditorToolkit::OnConvertVariablesToReroute()
{
	const FGraphPanelSelectionSet SelectedNodes = VoxelGraphEditor->GetSelectedNodes();
	if (SelectedNodes.Num() == 1)
	{
		for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(*NodeIt);
			if (GraphNode)
			{
				UEdGraph* Graph = GraphNode->GetGraph();
				const FScopedTransaction Transaction(VOXEL_LOCTEXT("Convert local variables to reroute"));
				Graph->Modify();

				UVoxelNode* CurrentSelectedNode = GraphNode->VoxelNode;
				UVoxelLocalVariableDeclaration* Declaration = Cast<UVoxelLocalVariableDeclaration>(CurrentSelectedNode);
				if (!Declaration)
				{
					UVoxelLocalVariableUsage* Usage = Cast<UVoxelLocalVariableUsage>(CurrentSelectedNode);
					if (Usage)
					{
						Declaration = Usage->Declaration;
					}
				}
				if (!Declaration)
				{
					return;
				}
				UEdGraphNode* DeclarationGraphNode = Declaration->GraphNode;

				FGraphNodeCreator<UVoxelGraphNode_Knot> KnotNodeCreator(*Graph);
				UVoxelGraphNode_Knot* KnotNode = KnotNodeCreator.CreateNode();
				KnotNodeCreator.Finalize();

				KnotNode->NodePosX = DeclarationGraphNode->NodePosX + 50;
				KnotNode->NodePosY = DeclarationGraphNode->NodePosY;

				for (UEdGraphPin* Pin : DeclarationGraphNode->GetAllPins())
				{
					if (Pin->Direction == EEdGraphPinDirection::EGPD_Input)
					{
						for (UEdGraphPin* InputPin : Pin->LinkedTo)
						{
							KnotNode->GetInputPin()->MakeLinkTo(InputPin);
						}
					}
					if (Pin->Direction == EEdGraphPinDirection::EGPD_Output)
					{
						for (UEdGraphPin* OutputPin : Pin->LinkedTo)
						{
							KnotNode->GetOutputPin()->MakeLinkTo(OutputPin);
						}
					}
				}
				DeclarationGraphNode->DestroyNode();

				for(UVoxelNode* Node : Generator->AllNodes)
				{
					auto* Usage = Cast<UVoxelLocalVariableUsage>(Node);
					if (Usage && Usage->Declaration == Declaration)
					{
						UEdGraphNode* UsageGraphNode = Usage->GraphNode;
						if (UsageGraphNode)
						{
							UEdGraphPin* Pin = Usage->GraphNode->GetAllPins()[0]; // usage node has a single pin
							for (UEdGraphPin* OutputPin : Pin->LinkedTo)
							{
								KnotNode->GetOutputPin()->MakeLinkTo(OutputPin);
							}
							UsageGraphNode->DestroyNode();
						}
					}
				}
				KnotNode->PropagatePinType();
			}
		}
	}
}

void FVoxelGraphEditorToolkit::ReconstructNode()
{
	const FGraphPanelSelectionSet SelectedNodes = VoxelGraphEditor->GetSelectedNodes();
	for(auto& Object : SelectedNodes)
	{
		if (auto* Node = Cast<UVoxelGraphNode>(Object))
		{
			Node->ReconstructNode();
		}
	}
	Generator->CompileVoxelNodesFromGraphNodes();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::RecreateNodes()
{
	for (int32 I = 0; I < 4; I++) // Hack to make sure they are really recreated
	{
		TArray<UVoxelGraphNode*> AllNodes;
		VoxelGraphEditor->GetCurrentGraph()->GetNodesOfClass<UVoxelGraphNode>(AllNodes);

		for (auto* Node : AllNodes)
		{
			Node->ReconstructNode();
		}

		Generator->CompileVoxelNodesFromGraphNodes();

		GraphTab->ClearContent();
		VoxelGraphEditor = CreateGraphEditorWidget(false);
		GraphTab->SetContent(VoxelGraphEditor.ToSharedRef());
	}
	ClearNodesMessages();
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::CompileToCpp()
{
	FVoxelMessages::Info("Compiling graphs to C++ requires Voxel Plugin Pro");
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::ToggleAutomaticPreview()
{
	Generator->Modify();
	Generator->bAutomaticPreview = !Generator->bAutomaticPreview;
}

bool FVoxelGraphEditorToolkit::IsToggleAutomaticPreviewChecked() const
{
	return Generator->bAutomaticPreview;
}

void FVoxelGraphEditorToolkit::UpdatePreview(EVoxelGraphPreviewFlags Flags)
{
	PreviewHandler->Update(Flags);

	if (EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::ManualPreview))
	{
		FVoxelMessages::Info("You can view and edit Voxel Graphs, but running and previewing them requires Voxel Plugin Pro");
	}
}

void FVoxelGraphEditorToolkit::UpdateVoxelWorlds()
{
	IVoxelEditorModule* VoxelEditorModule = &FModuleManager::LoadModuleChecked<IVoxelEditorModule>("VoxelEditor");
	VoxelEditorModule->RefreshVoxelWorlds(Generator);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::ClearNodesMessages()
{
	FVoxelGraphErrorReporter::ClearNodesMessages(Generator);
	ClearMessages(true, EVoxelGraphNodeMessageType::Info);
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::ShowAxisDependencies()
{
	FVoxelGraphErrorReporter::ClearNodesMessages(Generator);
	
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::UndoGraphAction()
{
	GEditor->UndoTransaction();
}

void FVoxelGraphEditorToolkit::RedoGraphAction()
{
	// Clear selection, to avoid holding refs to nodes that go away
	VoxelGraphEditor->ClearSelectionSet();

	GEditor->RedoTransaction();
}

///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphEditorToolkit::SelectNodeAndZoomToFit(TWeakObjectPtr<const UVoxelNode> Node)
{
	if (Node.IsValid() && Node->Graph)
	{
		if (Node->Graph == Generator)
		{
			SelectNodesAndZoomToFit({ Node->GraphNode });
		}
		else
		{
			if (ensure(GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Node->Graph)))
			{
				auto NewEditor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Node->Graph->VoxelGraph);
				if (ensure(NewEditor.IsValid()))
				{
					NewEditor->SelectNodesAndZoomToFit({ Node->GraphNode });
				}
			}
		}
	}
}