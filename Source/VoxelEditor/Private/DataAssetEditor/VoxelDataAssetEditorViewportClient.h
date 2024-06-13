// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"

class AVoxelWorld;
class FVoxelEditorToolsPanel;
class SVoxelDataAssetEditorViewport;
class FPreviewScene;
class UVoxelDataAsset;

class FVoxelDataAssetEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FVoxelDataAssetEditorViewportClient>
{
public:
	static TSharedRef<FVoxelDataAssetEditorViewportClient> Create(
		AVoxelWorld& VoxelWorld,
		UVoxelDataAsset& DataAsset,
		FVoxelEditorToolsPanel& Panel,
		FPreviewScene& PreviewScene,
		SVoxelDataAssetEditorViewport& DataAssetEditorViewport);

	//~ Begin FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(FViewport* Viewport, FInputDeviceId DeviceID, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual int32 GetCameraSpeedSetting() const override;
	virtual void SetCameraSpeedSetting(int32 SpeedSetting) override;
	virtual void MouseMove(FViewport* Viewport, int32 x, int32 y) override;
	virtual void UpdateMouseDelta() override;
	virtual UE::Widget::EWidgetMode GetWidgetMode() const override { return UE::Widget::EWidgetMode::WM_Max; }
	//~ End FEditorViewportClient interface

	bool IsShowGridToggled();
	void ToggleShowGrid();

private:
	AVoxelWorld& VoxelWorld;
	UVoxelDataAsset& DataAsset;
	FVoxelEditorToolsPanel& Panel;

	bool bShowGrid = false;
	bool bShowFloor = false;
	bool bMousePressed = false;
	
	FVoxelDataAssetEditorViewportClient(
		AVoxelWorld& VoxelWorld,
		UVoxelDataAsset& DataAsset,
		FVoxelEditorToolsPanel& Panel,
		FPreviewScene& InPreviewScene,
		SVoxelDataAssetEditorViewport& DataAssetEditorViewport);

	void ScheduleUpdateThumbnail();
	void UpdateThumbnail();
};