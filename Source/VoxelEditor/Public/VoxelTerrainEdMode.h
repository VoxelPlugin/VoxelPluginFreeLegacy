// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UnrealEd.h" 
#include "Editor.h"
#include "VoxelMarker.h"

class UVoxelTerrainEdModeData;

class FVoxelTerrainEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_VoxelTerrainEdModeId;

	FVoxelTerrainEdMode();

	TSharedRef<FUICommandList> GetUICommandList() const;
	
	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	/** FEdMode: Called when the mode is entered */
	virtual void Enter() override;

	/** FEdMode: Called when the mode is exited */
	virtual void Exit() override;

	/** FEdMode: Called when the mouse is moved over the viewport */
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY) override;

	/**
	* FEdMode: Called when the mouse is moved while a window input capture is in effect
	*
	* @param	InViewportClient	Level editor viewport client that captured the mouse input
	* @param	InViewport			Viewport that captured the mouse input
	* @param	InMouseX			New mouse cursor X coordinate
	* @param	InMouseY			New mouse cursor Y coordinate
	*
	* @return	true if input was handled
	*/
	virtual bool CapturedMouseMove(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

	/** FEdMode: Allow us to disable mouse delta tracking during painting */
	virtual bool DisallowMouseDeltaTracking() const override;

	/** FEdMode: Called once per frame */
	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	/** FEdMode: Called when clicking on a hit proxy */
	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click) override;

	/** FEdMode: Called when a key is pressed */
	virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override;

	/** FEdMode: Called when mouse drag input is applied */
	virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;

	/** FEdMode: Render elements for the landscape tool */
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;

	virtual bool UsesToolkits() const override;
	
	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed( AActor* InActor, bool bInSelection ) const override;
	// End of FEdMode interface

	void SetCurrentTool(FName Tool);
	void SetCurrentMode(FName Mode);

	FName GetCurrentTool() const;
	FName GetCurrentMode() const;

	void Undo();
	void Redo();

	UVoxelTerrainEdModeData* EdModeSettings;

private:
	FName CurrentToolName;
	FName CurrentModeName;

	EVoxelTool Tool;
	EVoxelMode Mode;

	AVoxelWorld* World;
	FVector HitPosition;
	FVector HitNormal;

	AVoxelSphereMarker* SphereMarker;
	AVoxelBoxMarker* BoxMarker;
	AVoxelProjectionMarker* ProjectionMarker;

	float LastProjectionEditTime;
	float Time;
	
	float SpotlightHeight = 400;
	float ToolHeight = 200;

	bool bMousePressed;
};