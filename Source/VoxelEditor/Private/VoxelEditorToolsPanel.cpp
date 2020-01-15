// Copyright 2020 Phyronnaz

#include "VoxelEditorToolsPanel.h"
#include "VoxelTools/VoxelToolManager.h"
#include "VoxelWorld.h"
#include "VoxelScopedTransaction.h"

#include "Editor.h"
#include "EditorViewportClient.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Layout/SScrollBox.h"

FVoxelEditorToolsPanel::FVoxelEditorToolsPanel()
{
}

FVoxelEditorToolsPanel::~FVoxelEditorToolsPanel()
{
	if (ToolManager)
	{
		ToolManager->Destroy();
	}
}

void FVoxelEditorToolsPanel::Init()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	ToolManager = NewObject<UVoxelToolManager>(GetTransientPackage(), NAME_None, RF_Transient);
	ToolManager->LoadConfig();
	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsPanel->SetIsPropertyVisibleDelegate(
		FIsPropertyVisible::CreateWeakLambda(
			ToolManager,
			[=](const FPropertyAndParent& PropertyAndParent)
			{
				return ToolManager->IsPropertyVisible(
					PropertyAndParent.Property,
#if ENGINE_MINOR_VERSION < 24
					PropertyAndParent.ParentProperty
#else
					PropertyAndParent.ParentProperties.Num() == 0 ? nullptr : PropertyAndParent.ParentProperties[0]
#endif
				);
			})
	);
	DetailsPanel->SetObject(ToolManager);
	DetailsPanel->OnFinishedChangingProperties().AddWeakLambda(ToolManager, [=](auto&) { ToolManager->SaveConfig(); });
	ToolManager->RefreshDetails.AddSP(DetailsPanel.ToSharedRef(), &IDetailsView::ForceRefresh);

	ToolManager->RegisterTransaction.AddLambda([](FName Name, AVoxelWorld* VoxelWorld)
	{
		FVoxelScopedTransaction Transaction(VoxelWorld, Name);
	});

	SAssignNew(Widget, SScrollBox)
	+ SScrollBox::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0)
		[
			DetailsPanel.ToSharedRef()
		]
	];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ToolManager);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelEditorToolsPanel::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
}

void FVoxelEditorToolsPanel::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (ToolManager)
	{
		FViewport* Viewport = ViewportClient->Viewport;

		// Make sure we have a valid viewport, otherwise we won't be able to construct an FSceneView
		if (Viewport != nullptr && Viewport->GetSizeXY().GetMin() > 0)
		{
			const int32 ViewportInteractX = Viewport->GetMouseX();
			const int32 ViewportInteractY = Viewport->GetMouseY();

			FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
				Viewport,
				ViewportClient->GetScene(),
				ViewportClient->EngineShowFlags)
				.SetRealtimeUpdate(ViewportClient->IsRealtime()));
			FSceneView* SceneView = ViewportClient->CalcSceneView(&ViewFamily);
			check(SceneView);
			
			const FViewportCursorLocation MouseViewportRay(SceneView, ViewportClient, ViewportInteractX, ViewportInteractY);

			FVector RayOrigin = MouseViewportRay.GetOrigin();
			const FVector RayDirection = MouseViewportRay.GetDirection();

			// If we're dealing with an orthographic view, push the origin of the ray backward along the viewport forward axis
			// to make sure that we can select objects that are behind the origin!
			if (!ViewportClient->IsPerspective())
			{
				RayOrigin -= RayDirection * WORLD_MAX / 2;
			}

			FVoxelToolManagerTickData TickData;
			TickData.World = ViewportClient->GetWorld();
			TickData.DeltaTime = DeltaTime;
			TickData.MousePosition = FVector2D(ViewportInteractX, ViewportInteractY);
			TickData.CameraViewDirection = SceneView->ViewMatrices.GetInvViewMatrix().TransformVector(FVector(0, 0, 1));
			TickData.RayOrigin = RayOrigin;
			TickData.RayDirection = RayDirection;
			TickData.bClick = bClick;
			TickData.bAlternativeMode = bAlternativeMode;
			TickData.MouseWheelDelta = MouseWheelDelta;
			ToolManager->Tick(TickData);
		}
	}

	MouseWheelDelta = 0;

	TimeUntilNextGC -= DeltaTime;
	if (TimeUntilNextGC <= 0)
	{
		TimeUntilNextGC = 30;
		UE_LOG(LogVoxel, Log, TEXT("Forcing GC"));
		GEngine->ForceGarbageCollection(true);
	}
}

void FVoxelEditorToolsPanel::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
}

bool FVoxelEditorToolsPanel::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{	
	if (Key == EKeys::LeftMouseButton)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			bClick = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			bClick = false;
		}
		return true;
	}
	else if (Key == EKeys::LeftShift || Key == EKeys::RightShift)
	{
		if (Event == EInputEvent::IE_Pressed)
		{
			bAlternativeMode = true;
		}
		else if (Event == EInputEvent::IE_Released)
		{
			bAlternativeMode = false;
		}
		return true;
	}
	else if (Key == EKeys::MouseScrollDown || Key == EKeys::MouseScrollUp)
	{
		return ViewportClient->IsPerspective();
	}
	else 
	{
		return false;
	}
}

bool FVoxelEditorToolsPanel::InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, float Delta, float DeltaTime)
{
	if (Key == EKeys::MouseWheelAxis && ViewportClient->IsPerspective())
	{
		MouseWheelDelta += Delta;
		return true;
	}

	return false;
}