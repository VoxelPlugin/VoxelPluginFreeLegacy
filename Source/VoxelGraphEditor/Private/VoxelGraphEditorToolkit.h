// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Misc/NotifyHook.h"
#include "EditorUndoClient.h"
#include "TickableEditorObject.h"
#include "Launch/Resources/Version.h"
#include "VoxelGraphErrorReporter.h"
#include "IVoxelGraphEditorToolkit.h"
#include "VoxelGlobals.h"

class IDetailsView;
class SVoxelGraphPreview;
class FVoxelGraphPreview;
class SVoxelPalette;
class SVoxelGraphPreviewViewport;
class SGraphEditor;
class SWidget;
class IMessageLogListing;

class FVoxelGraphEditorToolkit : public IVoxelGraphEditorToolkit, public FGCObject, public FNotifyHook, public FEditorUndoClient, public FTickableEditorObject
{
public:
	FVoxelGraphEditorToolkit();
	virtual ~FVoxelGraphEditorToolkit();

	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	void InitVoxelEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit);

private:
	// Creates all internal widgets for the tabs to point at
	void CreateInternalWidgets();

	// Add the toolbar buttons
	void FillToolbar(FToolBarBuilder& ToolbarBuilder);
	// Builds the toolbar. Calls FillToolbar
	void ExtendToolbar();

	// Fill the voxel menu dropdown
	void FillVoxelMenu(FMenuBuilder& MenuBuilder);
	// Adds additional dropdowns. Calls FillMenu
	void AddEditorMenus(FMenuBarBuilder& MenuBarBuilder);
	// Builds the menu. Calls AddEditorMenus
	void ExtendMenu();

	// Binds new graph commands to delegates
	void BindGraphCommands();

	// Create new graph editor widget
	TSharedRef<SGraphEditor> CreateGraphEditorWidget(bool bDebug);

public:
	//~ Begin IVoxelGraphEditorToolkit interface
	virtual bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) override;
	virtual int32 GetNumberOfSelectedNodes() const override;
	virtual TSet<UObject*> GetSelectedNodes() const override;
	virtual void SelectNodesAndZoomToFit(const TArray<UEdGraphNode*>& Nodes) override;
	virtual void RefreshNodesMessages() override;
	void TriggerUpdatePreview(bool bForce, bool bUpdateTextures) override;
	virtual FAdvancedPreviewScene* GetPreviewScene() const override;
	virtual void DebugNodes(const TSet<FVoxelCompilationNode*>& Nodes) override;
	virtual void AddMessages(const TArray<FVoxelGraphMessage>& Messages) override;
	virtual void ClearMessages(bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear) override;
	//~ End IVoxelGraphEditorToolkit interface

	//~ Begin IToolkit interface
	virtual FName GetToolkitFName() const override { return "VoxelGraphEditor"; }
	virtual FText GetBaseToolkitName() const override { return VOXEL_LOCTEXT("Voxel Graph Editor"); }
	virtual FString GetWorldCentricTabPrefix() const override { return "VoxelGraphEditor"; }
	virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f); }
	virtual void SaveAsset_Execute() override;
	//~ End IToolkit interface

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject interface

	//~ Begin FEditorUndoClient Interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }
	//~ End FEditorUndoClient Interface

	//~ Begin FNotifyHook Interface
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface

private:
	TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_DebugGraphCanvas(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Properties(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Shortcuts(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Palette(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PreviewViewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Messages(const FSpawnTabArgs& Args);

private:
	/**
	 * Graph events
	 */

	void OnSelectedNodesChanged(const TSet<UObject*>& NewSelection);

	// Called when a node's title is committed for a rename
	void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

	void OnNodeDoubleClicked(UEdGraphNode* Node);

	FReply OnSpawnGraphNodeByShortcut(FInputChord InChord, const FVector2D& InPosition);

public:
	/**
	 * Graph bindings
	 */

	void AddInput();
	bool CanAddInput() const;

	void DeleteInput();
	bool CanDeleteInput() const;

	void OnCreateComment();
	
	void OnTogglePinPreview();

	void SelectAllNodes();
	bool CanSelectAllNodes() const { return true; }

	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;
	void DeleteSelectedDuplicatableNodes(); // For cut

	void CutSelectedNodes();
	bool CanCutNodes() const;

	void CopySelectedNodes();
	bool CanCopyNodes() const;

	void PasteNodes();
	virtual void PasteNodesHere(const FVector2D& Location) override;
	virtual bool CanPasteNodes() const override;

	void DuplicateNodes();
	bool CanDuplicateNodes() const;
	
	void OnAlignTop();
	void OnAlignMiddle();
	void OnAlignBottom();
	void OnAlignLeft();
	void OnAlignCenter();
	void OnAlignRight();

	void OnStraightenConnections();

	void OnDistributeNodesH();
	void OnDistributeNodesV();

	void OnSelectLocalVariableDeclaration();
	void OnSelectLocalVariableUsages();
	void OnConvertRerouteToVariables();
	void OnConvertVariablesToReroute();

	void ReconstructNode();

public:
	/**
	 * Toolbar bindings
	 */

	void RecreateNodes();

	void CompileToCpp();
	
	void ToggleAutomaticPreview();
	bool IsToggleAutomaticPreviewChecked() const;
	void UpdatePreview(bool bUpdateTextures, bool bAutomaticPreview);
	void UpdateVoxelWorlds();
	
	void ClearNodesMessages();
	
	void ToggleRangeAnalysisDebug();
	bool IsRangeAnalysisDebugChecked();
	
	void ToggleStats();
	bool IsToggleStatsChecked() const;
	
	void ShowSelectedNodesStats();

	void ToggleShowAxisDependencies();
	void ShowAxisDependencies();
	bool IsShowAxisDependenciesChecked() const { return bShowAxisDependencies; }

	void ImportExposedVariablesValues();

	void UndoGraphAction();
	void RedoGraphAction();

public:
	// Message list action
	void SelectNodeAndZoomToFit(TWeakObjectPtr<const UVoxelNode> Node);

private:
	// The Voxel asset being inspected
	UVoxelGraphGenerator* WorldGenerator = nullptr;

	// Command list for this editor
	TSharedPtr<FUICommandList> GraphEditorCommands;

	bool bShowAxisDependencies = false;

	bool bUpdatePreviewOnNextTick = false;
	bool bNextPreviewUpdatesTextures = false;

	TArray<FVoxelGraphMessage> CurrentMessages;

private:
	/**
	 * Tabs
	 */
	
	// Graphs tabs
	TSharedPtr<SDockTab> GraphTab;
	TSharedPtr<SGraphEditor> VoxelGraphEditor;
	TSharedPtr<SDockTab> DebugGraphTab;
	TSharedPtr<SGraphEditor> VoxelDebugGraphEditor;

	// Properties tabs
	TSharedPtr<IDetailsView> VoxelProperties;
	TSharedPtr<IDetailsView> ShortcutsProperties;
	
	// Preview settings tab
	TSharedPtr<IDetailsView> PreviewSettings;
	
	// Palette of Voxel Node types
	TSharedPtr<SVoxelPalette> Palette;
	
	// Preview tab
	TSharedPtr<SVoxelGraphPreview> Preview;
	// 3D Preview
	TSharedPtr<SVoxelGraphPreviewViewport> PreviewViewport;

	// Preview handler
	TSharedPtr<FVoxelGraphPreview> PreviewHandler;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;

	// Messages panel
	TSharedPtr<SWidget> MessagesWidget;
	TSharedPtr<IMessageLogListing> MessagesListing;

	//	The tab ids for all the tabs used
	static const FName GraphCanvasTabId;
	static const FName DebugGraphCanvasTabId;
	static const FName PropertiesTabId;
	static const FName ShortcutsTabId;
	static const FName PreviewSettingsTabId;
	static const FName PaletteTabId;
	static const FName PreviewTabId;
	static const FName PreviewViewportTabId;
	static const FName MessagesTabId;
};