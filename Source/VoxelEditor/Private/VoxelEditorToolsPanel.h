// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Engine/EngineBaseTypes.h"
#include "VoxelGlobals.h"

class UVoxelToolManager;
class HHitProxy;
class SWidget;
class FViewport;
class FEditorViewportClient;
class IDetailsView;
struct FTransactionContext;
struct FViewportClick;
struct FKey;

class FVoxelEditorToolsPanel : public TSharedFromThis<FVoxelEditorToolsPanel>, public FGCObject
{	
public:
	FVoxelEditorToolsPanel();
	~FVoxelEditorToolsPanel();

	void Init();

public:
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface

public:
	inline const TSharedPtr<SWidget>& GetWidget() const
	{
		return Widget;
	}

public:
	void MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY);
	void Tick(FEditorViewportClient* ViewportClient, float DeltaTime);
	void HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);
	bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event);
	bool InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, float Delta, float DeltaTime);

private:
	UVoxelToolManager* ToolManager = nullptr;
	TSharedPtr<IDetailsView> DetailsPanel;
	TSharedPtr<SWidget> Widget;
	
	float TimeUntilNextGC = 0;

	bool bClick = false;
	bool bAlternativeMode = false;
	float MouseWheelDelta = 0;
};