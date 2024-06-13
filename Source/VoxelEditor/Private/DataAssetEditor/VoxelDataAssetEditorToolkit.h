// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "UObject/GCObject.h"
#include "Misc/NotifyHook.h"
#include "IVoxelDataAssetEditor.h"

class UVoxelDataAsset;
class FVoxelDataAssetEditorManager;
class IDetailsView;
class SWidget;
class FAdvancedPreviewScene;
class SVoxelDataAssetEditorViewport;
class FVoxelEditorToolsPanel;

class FVoxelDataAssetEditorToolkit : public IVoxelDataAssetEditor, public FGCObject, public FNotifyHook
{
public:
	FVoxelDataAssetEditorToolkit();
	virtual ~FVoxelDataAssetEditorToolkit();

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	void InitVoxelEditor(EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit);
	
private:
	// Creates all internal widgets for the tabs to point at
	void CreateInternalWidgets();
	// Builds the toolbar widget for the Voxel editor 
	void ExtendToolbar();
	// Called by ExtendToolbar
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	// Binds new graph commands to delegates 
	void BindCommands();

public:
	//~ Begin IToolkit interface
	virtual FName GetToolkitFName() const override { return "VoxelDataAssetEditor"; }
	virtual FText GetBaseToolkitName() const override { return VOXEL_LOCTEXT("Voxel Data Asset Editor"); }
	virtual FString GetWorldCentricTabPrefix() const override { return "VoxelDataAssetEditor"; }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }
	//~ End IToolkit interface

	//~ Begin FAssetEditorToolkit interface
	virtual void SaveAsset_Execute() override;
	virtual bool OnRequestClose() override;
	//~ End FAssetEditorToolkit interface

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return "FVoxelDataAssetEditorToolkit"; }
	//~ End FGCObject interface
	
	//~ Begin FNotifyHook interface
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook interface
	
	//~ Begin IVoxelDataAssetEditor interface
	virtual FAdvancedPreviewScene& GetPreviewScene() const override;
	virtual AVoxelWorld& GetVoxelWorld() const override;
	virtual UVoxelDataAsset& GetDataAsset() const override;
	virtual FVoxelEditorToolsPanel& GetPanel() const override;
	//~ End IVoxelDataAssetEditor interface
	
private:
	TSharedRef<SDockTab> SpawnTab_EditTools(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AdvancedPreviewSettings(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);

private:
	void InvertDataAsset();

private:
	// The Voxel asset being inspected
	TObjectPtr<UVoxelDataAsset> DataAsset = nullptr;
	
	// Manager, handles the voxel world
	TUniquePtr<FVoxelDataAssetEditorManager> Manager;

private:
	/**
	 * Tabs
	 */

	// Preview settings tab 
	TSharedPtr<IDetailsView> PreviewSettings;

	// Asset details
	TSharedPtr<IDetailsView> Details;

	// Advanced preview settings tab
	TSharedPtr<SWidget> AdvancedPreviewSettingsWidget;

	// Preview tab
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	TSharedPtr<SVoxelDataAssetEditorViewport> Preview;

	// Editor tools tab
	TSharedPtr<FVoxelEditorToolsPanel> ToolsPanel;
	
	//	The tab ids for all the tabs used
	static const FName EditToolsTabId;
	static const FName PreviewSettingsTabId;
	static const FName DetailsTabId;
	static const FName AdvancedPreviewSettingsTabId;
	static const FName PreviewTabId;
};