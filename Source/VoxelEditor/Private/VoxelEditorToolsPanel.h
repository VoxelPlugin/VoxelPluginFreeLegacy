// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "UObject/GCObject.h"
#include "Engine/EngineBaseTypes.h"
#include "Widgets/Input/SComboBox.h"

class AVoxelWorld;
class UVoxelToolManager;
class HHitProxy;
class SWidget;
class IDetailsView;
class FReply;
class FViewport;
class FSceneView;
class FUICommandList;
class FToolBarBuilder;
class FEditorViewportClient;
class FSceneViewFamilyContext;

struct FTransactionContext;
struct FViewportClick;
struct FKey;
struct EVisibility;

class FVoxelEditorToolsPanel : public TSharedFromThis<FVoxelEditorToolsPanel>, public FGCObject
{	
public:
	FVoxelEditorToolsPanel();
	~FVoxelEditorToolsPanel();

	void Init(const TSharedPtr<FUICommandList>& CommandListOverride = nullptr);
	void CustomizeToolbar(FToolBarBuilder& ToolBarBuilder);

public:
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

public:
	const TSharedRef<SBox>& GetWidget() const
	{
		return Widget;
	}

public:
	void MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY);
	void Tick(FEditorViewportClient* ViewportClient, float DeltaTime);
	void HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);
	bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event);
	bool InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, float Delta, float DeltaTime);

public:
	EVisibility GetAddVoxelWorldVisibility() const;
	FReply AddVoxelWorld() const;
	void ClearTool()
	{
		SetActiveTool(nullptr);
	}

private:
	UVoxelToolManager* ToolManager = nullptr;

	TSharedPtr<IDetailsView> SharedConfigDetailsPanel;
	TSharedPtr<IDetailsView> ToolDetailsPanel;

	TSharedPtr<SComboBox<TSharedPtr<UClass*>>> ComboBox;
	TSharedRef<SBox> Widget;
	
	TWeakObjectPtr<UWorld> LastWorld;
	TWeakObjectPtr<AVoxelWorld> LastVoxelWorld;

	TArray<TSharedPtr<UClass*>> ToolsOptions;
	TSharedPtr<FUICommandList> CommandList;
	
	float TimeUntilNextGC = 0;

	bool bClick = false;
	bool bAlternativeMode = false;

	float BrushSizeDelta = 0.f;
	float FalloffDelta = 0.f;
	float StrengthDelta = 0.f;

	bool bShowCustomTools = false;
	TSharedPtr<SButton> ExpanderButton;

private:
	void RefreshDetails() const;
	bool IsPropertyVisible(const FProperty& Property, const TArray<const FProperty*>& ParentProperties, int32 ParentPropertyIndex = 0) const;

private:
	void SetActiveTool(UClass* ToolClass);
	bool IsToolActive(UClass* ToolClass) const;
	void BuildToolBars(TArray<FToolBarBuilder>& OutToolBars, TArray<FToolBarBuilder>& OutCustomToolBars);
	void BindCommands();

private:
	// Only valid in Tick
	FEditorViewportClient* ViewportClientForDeproject = nullptr;

	FSceneView* GetSceneView(TUniquePtr<FSceneViewFamilyContext>& ViewFamily) const;
	bool Deproject(const FVector2D& ScreenPosition, FVector& OutWorldPosition, FVector& OutWorldDirection) const;
};