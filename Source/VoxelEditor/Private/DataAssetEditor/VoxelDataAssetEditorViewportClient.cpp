// Copyright 2021 Phyronnaz

#include "DataAssetEditor/VoxelDataAssetEditorViewportClient.h"
#include "DataAssetEditor/SVoxelDataAssetEditorViewport.h"
#include "VoxelEditorToolsPanel.h"
#include "VoxelWorldEditorControls.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorld.h"
#include "VoxelIntBox.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Components/LineBatchComponent.h"
#include "Engine/World.h"
#include "PreviewScene.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "TimerManager.h"

TSharedRef<FVoxelDataAssetEditorViewportClient> FVoxelDataAssetEditorViewportClient::Create(
	AVoxelWorld& VoxelWorld, 
	UVoxelDataAsset& DataAsset, 
	FVoxelEditorToolsPanel& Panel, 
	FPreviewScene& PreviewScene, 
	SVoxelDataAssetEditorViewport& DataAssetEditorViewport)
{
	const auto Result = TSharedRef<FVoxelDataAssetEditorViewportClient>(new FVoxelDataAssetEditorViewportClient(
		VoxelWorld,
		DataAsset,
		Panel,
		PreviewScene,
		DataAssetEditorViewport));

	check(VoxelWorld.IsCreated());
	VoxelWorld.GetRuntime().RuntimeData->OnWorldLoaded.AddSP(Result, &FVoxelDataAssetEditorViewportClient::ScheduleUpdateThumbnail);

	return Result;
}

FVoxelDataAssetEditorViewportClient::FVoxelDataAssetEditorViewportClient(
	AVoxelWorld& VoxelWorld, 
	UVoxelDataAsset& DataAsset, 
	FVoxelEditorToolsPanel& Panel, 
	FPreviewScene& InPreviewScene, 
	SVoxelDataAssetEditorViewport& DataAssetEditorViewport)
	: FEditorViewportClient(nullptr, &InPreviewScene, StaticCastSharedRef<SEditorViewport>(DataAssetEditorViewport.AsShared()))
	, VoxelWorld(VoxelWorld)
	, DataAsset(DataAsset)
	, Panel(Panel)
{
	Widget->SetSnapEnabled(true);
	DrawHelper.bDrawGrid = bShowGrid;

	{
		// View the asset so that Camera Forward = Min Axis
		
		const FVoxelIntBox Bounds = DataAsset.GetBounds();
		const FIntVector Size = Bounds.Size();

		const int32 Axis = FVoxelUtilities::GetArgMin(Size);

		// Max of vertical and horizontal size of the viewed object
		const float MaxSize = FMath::Max(Size[(Axis + 1) % 3], Size[(Axis + 2) % 3]);

		const float HalfHorizontalFOV = FMath::DegreesToRadians(ViewFOV / 2);
		const float HalfVerticalFOV = FMath::Atan(FMath::Tan(HalfHorizontalFOV) / AspectRatio);

		// Distance required to fit the object into view
		const float Distance = (MaxSize / 2) / FMath::Tan(FMath::Min(HalfVerticalFOV, HalfHorizontalFOV));
		
		FVector Position = Bounds.GetCenter();
		// Be far enough to have the asset in view
		Position[Axis] = Size[Axis] + Distance;
		SetViewLocation(Position * VoxelWorld.VoxelSize);

		FVector Direction = FVector::ZeroVector;
		Direction[Axis] = -1; // negative so that if the axis is Z we look down and not up
		SetViewRotation(Direction.ToOrientationRotator());
	}
	
	SetRealtime(true);
	bSetListenerPosition = false;

	EngineShowFlags.EnableAdvancedFeatures();
	EngineShowFlags.SetLighting(true);
	EngineShowFlags.SetIndirectLightingCache(true);
	EngineShowFlags.SetPostProcessing(true);
	EngineShowFlags.SetSelectionOutline(false);
	
	Invalidate();
}

void FVoxelDataAssetEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	if (!ensure(VoxelWorld.IsCreated())) return;

	UWorld* World = VoxelWorld.GetWorld();
	for (auto* VoxelWorldEditor : TActorRange<AVoxelWorldEditorControls>(World))
	{
		VoxelWorldEditor->bOverrideLocation = true;
		VoxelWorldEditor->LocationOverride = GetViewLocation();
	}

	World->Tick(LEVELTICK_All, DeltaSeconds);
	Panel.Tick(this, DeltaSeconds);

	if (VoxelWorld.GetSubsystemChecked<FVoxelData>().IsDirty())
	{
		DataAsset.MarkPackageDirty();
	}
}

void FVoxelDataAssetEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);
}

void FVoxelDataAssetEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
	TArray<FDebugDisplayProperty> EmptyPropertyArray;
	DrawStatsHUD(VoxelWorld.GetWorld(), InViewport, Canvas, nullptr, EmptyPropertyArray, GetViewLocation(), GetViewRotation());
}

bool FVoxelDataAssetEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	bool bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, InViewport, Key, Event);;

	if (Key == EKeys::MouseScrollDown || Key == EKeys::MouseScrollUp)
	{
		bHandled = true;
	}

	if (Key == EKeys::LeftMouseButton && Event != EInputEvent::IE_Repeat)
	{
		bMousePressed = (Event == EInputEvent::IE_Pressed);
	}

	if (Event == IE_Released && (Key == EKeys::LeftMouseButton || Key == EKeys::MiddleMouseButton || Key == EKeys::RightMouseButton))
	{
		//Set the cursor position to that of the slate cursor so it wont snap back
		Viewport->SetPreCaptureMousePosFromSlateCursor();
	}

	bHandled &= Panel.InputKey(this, InViewport, Key, Event);

	if (!bHandled)
	{
		bHandled = FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
	}

	return bHandled;
}

bool FVoxelDataAssetEditorViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	return Panel.InputAxis(this, InViewport, Key, Delta, DeltaTime) || FEditorViewportClient::InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
}

void FVoxelDataAssetEditorViewportClient::ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);
	GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click);
	Panel.HandleClick(this, HitProxy, Click);
}

int32 FVoxelDataAssetEditorViewportClient::GetCameraSpeedSetting() const
{
	return GetDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed;
}

void FVoxelDataAssetEditorViewportClient::SetCameraSpeedSetting(int32 SpeedSetting)
{
	GetMutableDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed = SpeedSetting;
}

void FVoxelDataAssetEditorViewportClient::MouseMove(FViewport* InViewport, int32 x, int32 y)
{
	FEditorViewportClient::MouseMove(InViewport, x, y);

	Panel.MouseMove(this, InViewport, x, y);
}

void FVoxelDataAssetEditorViewportClient::UpdateMouseDelta()
{
	if (!bMousePressed)
	{
		FEditorViewportClient::UpdateMouseDelta();
	}
}

bool FVoxelDataAssetEditorViewportClient::IsShowGridToggled() 
{
	return bShowGrid;
}

void FVoxelDataAssetEditorViewportClient::ToggleShowGrid() 
{
	bShowGrid = !bShowGrid;
	DrawHelper.bDrawGrid = bShowGrid;
	Invalidate();
}

void FVoxelDataAssetEditorViewportClient::ScheduleUpdateThumbnail()
{
	// Wait a bit to make sure chunks are rendered
	FTimerHandle Handle;
	auto& TimerManager = VoxelWorld.GetWorldTimerManager();
	TimerManager.SetTimer(Handle, FTimerDelegate::CreateSP(this, &FVoxelDataAssetEditorViewportClient::UpdateThumbnail), 0.5, false);
}

void FVoxelDataAssetEditorViewportClient::UpdateThumbnail()
{
	uint32 SrcWidth = Viewport->GetSizeXY().X;
	uint32 SrcHeight = Viewport->GetSizeXY().Y;
	
	// Read the contents of the viewport into an array.
	TArray<FColor> OrigBitmap;
	if (Viewport->ReadPixels(OrigBitmap))
	{
		check(OrigBitmap.Num() == SrcWidth * SrcHeight);

		// Pin to smallest value
		int32 CropSize = FMath::Min<uint32>(SrcWidth, SrcHeight);

		// Calculations for cropping
		TArray<FColor> CroppedBitmap;
		CroppedBitmap.AddUninitialized(CropSize * CropSize);

		// Crop the image
		int32 CroppedSrcTop = (SrcHeight - CropSize) / 2;
		int32 CroppedSrcLeft = (SrcWidth - CropSize) / 2;

		for (int32 Row = 0; Row < CropSize; Row++)
		{
			// Row*Side of a row*byte per color
			int32 SrcPixelIndex = (CroppedSrcTop + Row) * SrcWidth + CroppedSrcLeft;
			const void* SrcPtr = &(OrigBitmap[SrcPixelIndex]);
			void* DstPtr = &(CroppedBitmap[Row * CropSize]);
			FMemory::Memcpy(DstPtr, SrcPtr, CropSize * 4);
		}

		// Scale image down if needed
		TArray<FColor> ScaledBitmap;
		if (DATA_ASSET_THUMBNAIL_RES != CropSize)
		{
			FImageUtils::ImageResize(CropSize, CropSize, CroppedBitmap, DATA_ASSET_THUMBNAIL_RES, DATA_ASSET_THUMBNAIL_RES, ScaledBitmap, true);
		}
		else
		{
			//just copy the data over. sizes are the same
			ScaledBitmap = CroppedBitmap;
		}

		DataAsset.SetThumbnail(MoveTemp(ScaledBitmap));
	}
}