// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "SVoxelGraphPreviewViewport.h"
#include "IVoxelGraphEditorToolkit.h"
#include "VoxelDefinitions.h"

#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"
#include "Slate/SceneViewport.h"
#include "AssetViewerSettings.h"
#include "UnrealWidget.h"

/** Viewport Client for the preview viewport */
class FVoxelGraphEditorViewportClient : public FEditorViewportClient
{
public:
	FVoxelGraphEditorViewportClient(FAdvancedPreviewScene* InPreviewScene, const TSharedRef<SVoxelGraphPreviewViewport>& InVoxelGraphEditorViewport);

	// FEditorViewportClient interface
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
#if VOXEL_ENGINE_VERSION >= 506
	virtual bool InputAxis(const FInputKeyEventArgs& Args) override;
#else
	virtual bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples/* =1 */, bool bGamepad/* =false */) override;
#endif
	virtual FLinearColor GetBackgroundColor() const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas) override;
	virtual bool ShouldOrbitCamera() const override;

private:

	/** Preview Scene - uses advanced preview settings */
	class FAdvancedPreviewScene* AdvancedPreviewScene;
};

FVoxelGraphEditorViewportClient::FVoxelGraphEditorViewportClient(FAdvancedPreviewScene* InPreviewScene, const TSharedRef<SVoxelGraphPreviewViewport>& InVoxelGraphEditorViewport)
	: FEditorViewportClient(nullptr, InPreviewScene, StaticCastSharedRef<SEditorViewport>(InVoxelGraphEditorViewport))
{
	// Setup defaults for the common draw helper.
	DrawHelper.bDrawPivot = false;
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawKillZ = false;
	DrawHelper.bDrawGrid = false;
	DrawHelper.GridColorAxis = FColor(80, 80, 80);
	DrawHelper.GridColorMajor = FColor(72, 72, 72);
	DrawHelper.GridColorMinor = FColor(64, 64, 64);
	DrawHelper.PerspectiveGridSize = HALF_WORLD_MAX1;

	SetViewMode(VMI_Lit);

	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSnap(false);
	EngineShowFlags.SetSeparateTranslucency(true);

	OverrideNearClipPlane(1.0f);
	bUsingOrbitCamera = true;

	// Don't want to display the widget in this viewport
	Widget->SetDefaultVisibility(false);

	AdvancedPreviewScene = InPreviewScene;

}

void FVoxelGraphEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread)
	{
		PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}

void FVoxelGraphEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}

bool FVoxelGraphEditorViewportClient::ShouldOrbitCamera() const
{
	// Should always orbit around the preview object to keep it in view.
	return true;
}

bool FVoxelGraphEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	bool bHandled = FEditorViewportClient::InputKey(EventArgs);

	// Handle viewport screenshot.
	bHandled |= InputTakeScreenshot(EventArgs.Viewport, EventArgs.Key, EventArgs.Event);

	bHandled |= AdvancedPreviewScene->HandleInputKey(EventArgs);

	return bHandled;
}

#if VOXEL_ENGINE_VERSION >= 506
bool FVoxelGraphEditorViewportClient::InputAxis(const FInputKeyEventArgs& Args)
{
	bool bResult = true;

	if (!bDisableInput)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		bResult = AdvancedPreviewScene->HandleViewportInput(Args.Viewport, Args.InputDevice, Args.Key, Args.AmountDepressed, Args.DeltaTime, Args.NumSamples, Args.IsGamepad());
		if (bResult)
		{
			Invalidate();
		}
		else
		{
			bResult = FEditorViewportClient::InputAxis(Args);
		}
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	return bResult;
}
#else
bool FVoxelGraphEditorViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples/* =1 */, bool bGamepad/* =false */)
{
	bool bResult = true;

	if (!bDisableInput)
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		bResult = AdvancedPreviewScene->HandleViewportInput(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
		if (bResult)
		{
			Invalidate();
		}
		else
		{
			bResult = FEditorViewportClient::InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
		}
		PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	return bResult;
}
#endif

FLinearColor FVoxelGraphEditorViewportClient::GetBackgroundColor() const
{
	if (AdvancedPreviewScene != nullptr)
	{
		return AdvancedPreviewScene->GetBackgroundColor();
	}
	else
	{
		return FLinearColor::White;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SVoxelGraphPreviewViewport::Construct(const FArguments& InArgs)
{
	VoxelGraphEditorToolkit = InArgs._VoxelGraphEditorToolkit;
	AdvancedPreviewScene = VoxelGraphEditorToolkit.Pin()->GetPreviewScene();

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

SVoxelGraphPreviewViewport::~SVoxelGraphPreviewViewport()
{
	UAssetViewerSettings::Get()->OnAssetViewerSettingsChanged().RemoveAll(this);

	if (EditorViewportClient.IsValid())
	{
		EditorViewportClient->Viewport = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<class SEditorViewport> SVoxelGraphPreviewViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SVoxelGraphPreviewViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SVoxelGraphPreviewViewport::OnFloatingButtonClicked()
{
}

void SVoxelGraphPreviewViewport::RefreshViewport()
{
	Client->RedrawRequested(nullptr);
	GEditor->UpdateSingleViewportClient(Client.Get(), true, false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TSharedRef<FEditorViewportClient> SVoxelGraphPreviewViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FVoxelGraphEditorViewportClient(AdvancedPreviewScene, SharedThis(this)));
	EditorViewportClient->SetViewLocation(FVector::ZeroVector);
	EditorViewportClient->SetViewRotation(FRotator(0.0f, -90.0f, 0.0f));
	EditorViewportClient->SetViewLocationForOrbiting(FVector::ZeroVector);

	return EditorViewportClient.ToSharedRef();
}