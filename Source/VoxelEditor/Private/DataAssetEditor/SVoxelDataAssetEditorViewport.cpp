// Copyright 2020 Phyronnaz

#include "DataAssetEditor/SVoxelDataAssetEditorViewport.h"
#include "DataAssetEditor/VoxelDataAssetEditorToolkit.h"
#include "DataAssetEditor/VoxelDataAssetEditorViewportClient.h"

#include "AssetViewerSettings.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"
#include "EditorViewportClient.h"
#include "Slate/SceneViewport.h"
#include "Widgets/Docking/SDockTab.h"
#include "VoxelDataAssetEditorCommands.h"

void SVoxelDataAssetEditorViewportToolBar::Construct(const FArguments& InArgs, TSharedPtr<class SVoxelDataAssetEditorViewport> InViewport)
{
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InViewport);
}

TSharedRef<SWidget> SVoxelDataAssetEditorViewportToolBar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();

	const TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		auto& Commands = FVoxelDataAssetEditorCommands::Get();

		ShowMenuBuilder.AddMenuEntry(Commands.TogglePreviewGrid);
		ShowMenuBuilder.AddMenuEntry(Commands.TogglePreviewBackground);
	}

	return ShowMenuBuilder.MakeWidget();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelDataAssetEditorViewport::Construct(const FArguments& InArgs)
{
	DataAssetEditor = InArgs._Editor;
	check(DataAssetEditor);
	AdvancedPreviewScene = &DataAssetEditor->GetPreviewScene();
	
	SEditorViewport::Construct( SEditorViewport::FArguments() );
}

SVoxelDataAssetEditorViewport::~SVoxelDataAssetEditorViewport()
{
	UAssetViewerSettings::Get()->OnAssetViewerSettingsChanged().RemoveAll(this);
	
	if (EditorViewportClient.IsValid())
	{
		EditorViewportClient->Viewport = nullptr;
	}
}

void SVoxelDataAssetEditorViewport::RefreshViewport()
{
	SceneViewport->InvalidateDisplay();

	if (EditorViewportClient.IsValid())
	{
		UAssetViewerSettings* Settings = UAssetViewerSettings::Get();
		const int32 ProfileIndex = AdvancedPreviewScene->GetCurrentProfileIndex();
		if (Settings->Profiles.IsValidIndex(ProfileIndex))
		{
			AdvancedPreviewScene->UpdateScene(Settings->Profiles[ProfileIndex]);
		}
	}
}

bool SVoxelDataAssetEditorViewport::IsVisible() const
{
	return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground()) && SEditorViewport::IsVisible();
}

void SVoxelDataAssetEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FVoxelDataAssetEditorCommands& Commands = FVoxelDataAssetEditorCommands::Get();

	CommandList->Append(DataAssetEditor->GetToolkitCommands());

	CommandList->MapAction(
		Commands.TogglePreviewGrid,
		FExecuteAction::CreateSP(EditorViewportClient.ToSharedRef(), &FVoxelDataAssetEditorViewportClient::ToggleShowGrid),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(EditorViewportClient.ToSharedRef(), &FVoxelDataAssetEditorViewportClient::IsShowGridToggled));

	CommandList->MapAction(
		Commands.TogglePreviewBackground,
		FExecuteAction::CreateSP(this, &SVoxelDataAssetEditorViewport::TogglePreviewBackground),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SVoxelDataAssetEditorViewport::IsPreviewBackgroundToggled));
}

void SVoxelDataAssetEditorViewport::TogglePreviewBackground()
{
	UAssetViewerSettings* Settings = UAssetViewerSettings::Get();
	const int32 ProfileIndex = AdvancedPreviewScene->GetCurrentProfileIndex();
	if (Settings->Profiles.IsValidIndex(ProfileIndex))
	{
		AdvancedPreviewScene->SetEnvironmentVisibility(!Settings->Profiles[ProfileIndex].bShowEnvironment);
	}
	RefreshViewport();
}

bool SVoxelDataAssetEditorViewport::IsPreviewBackgroundToggled() const
{
	UAssetViewerSettings* Settings = UAssetViewerSettings::Get();
	const int32 ProfileIndex = AdvancedPreviewScene->GetCurrentProfileIndex();
	if (Settings->Profiles.IsValidIndex(ProfileIndex))
	{
		return Settings->Profiles[ProfileIndex].bShowEnvironment;
	}
	return false;
}

TSharedRef<SEditorViewport> SVoxelDataAssetEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SVoxelDataAssetEditorViewport::GetExtenders() const
{
	return MakeShared<FExtender>();
}

TSharedRef<FEditorViewportClient> SVoxelDataAssetEditorViewport::MakeEditorViewportClient() 
{
	EditorViewportClient = FVoxelDataAssetEditorViewportClient::Create(
		DataAssetEditor->GetVoxelWorld(),
		DataAssetEditor->GetDataAsset(),
		DataAssetEditor->GetPanel(),
		*AdvancedPreviewScene, 
		*this);
	
	EditorViewportClient->VisibilityDelegate.BindSP(this, &SVoxelDataAssetEditorViewport::IsVisible);

	return EditorViewportClient.ToSharedRef();
}

void SVoxelDataAssetEditorViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay)
{
	Overlay->AddSlot()
	.VAlign(VAlign_Top)
	[
		SNew(SVoxelDataAssetEditorViewportToolBar, SharedThis(this))
	];
}