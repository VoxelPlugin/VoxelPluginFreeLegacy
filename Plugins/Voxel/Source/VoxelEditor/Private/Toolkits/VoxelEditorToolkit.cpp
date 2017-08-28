// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "VoxelEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "SVoxelEditor.h"
#include "VoxelAsset.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FVoxelEditorToolkit"

DEFINE_LOG_CATEGORY_STATIC(LogVoxelEditor, Log, All);


/* Local constants
 *****************************************************************************/

namespace VoxelEditor
{
	static const FName AppIdentifier("VoxelEditorApp");
	static const FName TabId("TextEditor");
}


/* FVoxelEditorToolkit structors
 *****************************************************************************/

FVoxelEditorToolkit::FVoxelEditorToolkit(const TSharedRef<ISlateStyle>& InStyle)
	: Voxel(nullptr)
	, Style(InStyle)
{ }


FVoxelEditorToolkit::~FVoxelEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}


/* FVoxelEditorToolkit interface
 *****************************************************************************/

void FVoxelEditorToolkit::Initialize(UVoxelAsset* InVoxel, const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost)
{
	Voxel = InVoxel;

	// Support undo/redo
	Voxel->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_VoxelEditor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
						->SetOrientation(Orient_Vertical)
						->SetSizeCoefficient(0.66f)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
								
						)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(VoxelEditor::TabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.9f)
						)
				)
		);

	FAssetEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		VoxelEditor::AppIdentifier,
		Layout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InVoxel
	);

	RegenerateMenusAndToolbars();
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FVoxelEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://github.com/Phyronnaz/MarchingCubes"));
}


void FVoxelEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_VoxelEditor", "Voxel Asset Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(VoxelEditor::TabId, FOnSpawnTab::CreateSP(this, &FVoxelEditorToolkit::HandleTabManagerSpawnTab, VoxelEditor::TabId))
		.SetDisplayName(LOCTEXT("VoxelEditorTabName", "Voxel Editor"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}


void FVoxelEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(VoxelEditor::TabId);
}


/* IToolkit interface
 *****************************************************************************/

FText FVoxelEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Voxel Asset Editor");
}


FName FVoxelEditorToolkit::GetToolkitFName() const
{
	return FName("VoxelEditor");
}


FLinearColor FVoxelEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FVoxelEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Voxel ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FVoxelEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Voxel);
}


/* FEditorUndoClient interface
*****************************************************************************/

void FVoxelEditorToolkit::PostUndo(bool bSuccess)
{ }


void FVoxelEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}


/* FVoxelEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FVoxelEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	if (TabIdentifier == VoxelEditor::TabId)
	{
		TabWidget = SNew(SVoxelEditor, Voxel, Style);
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}


#undef LOCTEXT_NAMESPACE
