// Copyright 2020 Phyronnaz

#include "VoxelEditorToolsPanel.h"
#include "VoxelTools/VoxelToolManager.h"
#include "VoxelWorld.h"
#include "VoxelScopedTransaction.h"
#include "ActorFactoryVoxelWorld.h"

#include "EngineUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

FVoxelEditorToolsPanel::FVoxelEditorToolsPanel()
{
}

FVoxelEditorToolsPanel::~FVoxelEditorToolsPanel()
{
	if (ToolManager)
	{
		ToolManager->ClearToolInstance();
	}
}

void FVoxelEditorToolsPanel::Init()
{	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	ToolManager = NewObject<UVoxelToolManager>(GetTransientPackage(), NAME_None, RF_Transient | RF_Transactional);
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
		.Padding(FMargin(3, 5))
		.AutoHeight()
		[
			SNew(SBorder)
			.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FVoxelEditorToolsPanel::GetAddVoxelWorldVisibility)))
			.BorderBackgroundColor(FLinearColor::Red)
			.BorderImage(FCoreStyle::Get().GetBrush("ErrorReporting.Box"))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(3, 0))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",12))
					.Text(VOXEL_LOCTEXT("No Voxel World in the scene!"))
				]
				+ SVerticalBox::Slot()
				.Padding(FMargin(0, 5))
				[
					SNew(SButton)
					.OnClicked(FOnClicked::CreateSP(this, &FVoxelEditorToolsPanel::AddVoxelWorld))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::Black)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular",12))
						.Text(VOXEL_LOCTEXT("Add Voxel World"))
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.Padding(0)
		.FillHeight(1)
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
	auto* World = ViewportClient->GetWorld();
	if (!ensure(World)) return;
	
	if (!LastWorld.IsValid())
	{
		// Toggle voxel worlds if none are created on first tick
		const auto ToggleVoxelWorld = [&]()
		{
			for (auto* VoxelWorld : TActorRange<AVoxelWorld>(World))
			{
				if (VoxelWorld->IsCreated())
				{
					return;
				}
			}
			for (auto* VoxelWorld : TActorRange<AVoxelWorld>(World))
			{
				if (!VoxelWorld->IsCreated())
				{
					VoxelWorld->Toggle();
				}
			}
		};
		ToggleVoxelWorld();
	}
	LastWorld = ViewportClient->GetWorld();
	
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
		LOG_VOXEL(Log, TEXT("Forcing GC"));
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

EVisibility FVoxelEditorToolsPanel::GetAddVoxelWorldVisibility() const
{
	if (!LastWorld.IsValid())
	{
		return EVisibility::Collapsed;
	}
	
	for (auto* VoxelWorld : TActorRange<AVoxelWorld>(LastWorld.Get()))
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

FReply FVoxelEditorToolsPanel::AddVoxelWorld() const
{
	if (!LastWorld.IsValid())
	{
		return FReply::Handled();
	}

	auto* Factory = NewObject<UActorFactoryVoxelWorld>();
	if (ensure(Factory))
	{
		Factory->CreateActor(nullptr, LastWorld->GetLevel(0), FTransform::Identity);
	}

	return FReply::Handled();
}