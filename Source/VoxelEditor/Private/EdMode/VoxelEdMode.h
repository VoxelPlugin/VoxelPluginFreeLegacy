// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Editor.h"
#include "EdMode.h"

class AVoxelWorld;
class FEdModeVoxel;
class FVoxelEditorToolsPanel;
struct FViewportClick;

class FEdModeVoxel : public FEdMode
{
public:
	const static FEditorModeID EM_Voxel;
	
public:
	virtual void Enter() override;
	virtual void Exit() override;

public:
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY) override;
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	virtual bool HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
	virtual bool InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime) override;

public:
	virtual bool CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY) override;
	virtual bool DisallowMouseDeltaTracking() const override;
	virtual bool UsesToolkits() const override { return true; }
	virtual bool IsSelectionAllowed( AActor* InActor, bool bInSelection ) const override;	
	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const override;

public:
	FVoxelEditorToolsPanel& GetPanel() const { return *Panel; }

private:
	bool bMousePressed = false;
	TSharedPtr<FVoxelEditorToolsPanel> Panel;
};