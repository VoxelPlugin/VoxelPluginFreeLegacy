// Copyright 2018 Phyronnaz

#include "VoxelTerrainEdMode.h"
#include "VoxelTerrainEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "VoxelTerrainEdModeData.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "VoxelTools.h"
#include "VoxelChunksOwner.h"
#include "VoxelTerrainEdModeCommands.h"

const FEditorModeID FVoxelTerrainEdMode::EM_VoxelTerrainEdModeId = TEXT("EM_VoxelTerrainEdMode");

FVoxelTerrainEdMode::FVoxelTerrainEdMode()
	: World(nullptr)
	, SphereMarker(nullptr)
	, BoxMarker(nullptr)
	, ProjectionMarker(nullptr)
	, CurrentModeName("EditMode")
	, CurrentToolName("ProjectionTool")
	, Mode(EVoxelMode::Edit)
	, Tool(EVoxelTool::Projection)
	, LastProjectionEditTime(0)
	, Time(0)
	, bMousePressed(false)
	, LastForceGCTime(0)
{
	EdModeSettings = NewObject<UVoxelTerrainEdModeData>(GetTransientPackage(), TEXT("VoxelTerrainEdModeData"), RF_Transactional);
}

TSharedRef<FUICommandList> FVoxelTerrainEdMode::GetUICommandList() const
{
	check(Toolkit.IsValid());
	return Toolkit->GetToolkitCommands();
}

/** FGCObject interface */
void FVoxelTerrainEdMode::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);

	Collector.AddReferencedObject(EdModeSettings);
}

void FVoxelTerrainEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FVoxelTerrainEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FVoxelTerrainEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	if (SphereMarker)
	{
		SphereMarker->Destroy();
		SphereMarker = nullptr;
	}
	if (BoxMarker)
	{
		BoxMarker->Destroy();
		BoxMarker = nullptr;
	}
	if (ProjectionMarker)
	{
		ProjectionMarker->Destroy();
		ProjectionMarker = nullptr;
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FVoxelTerrainEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	if (!IsEnabled())
	{
		return false;
	}

	FVector Start, End;
	{
		// Compute a world space ray from the screen space mouse coordinates
		FSceneViewFamilyContext ViewFamily(FSceneViewFamilyContext::ConstructionValues(ViewportClient->Viewport, ViewportClient->GetScene(), ViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(ViewportClient->IsRealtime()));

		FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);
		FViewportCursorLocation MouseViewportRay(View, ViewportClient, MouseX, MouseY);
		FVector MouseViewportRayDirection = MouseViewportRay.GetDirection();

		Start = MouseViewportRay.GetOrigin();
		End = Start + WORLD_MAX * MouseViewportRayDirection;
		if (ViewportClient->IsOrtho())
		{
			Start -= WORLD_MAX * MouseViewportRayDirection;
		}
	}

	World = nullptr;
	{
		TArray<FHitResult> HitResults;
		if (GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_Visibility))
		{
			for (int Index = 0; Index < HitResults.Num(); Index++)
			{
				auto& HitResult = HitResults[Index];
				HitPosition = HitResult.ImpactPoint;
				HitNormal = HitResult.ImpactNormal;

				if (HitResult.Actor->IsA(AVoxelChunksOwner::StaticClass()))
				{
					World = Cast<AVoxelChunksOwner>(HitResult.Actor.Get())->World;
				}
			}
		}
	}

	switch (Tool)
	{
	case EVoxelTool::Projection:
	{
		SphereMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));
		BoxMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));

		ProjectionMarker->SetParametersCpp(World, Mode, HitPosition, HitNormal, SpotlightHeight, ToolHeight, EdModeSettings->Radius, ViewportClient->Viewport->KeyState(EdModeSettings->Add));
		break;
	}
	case EVoxelTool::Sphere:
	{
		BoxMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));
		ProjectionMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));

		SphereMarker->SetParametersCpp(World, Mode, HitPosition, HitNormal, SpotlightHeight, ToolHeight, EdModeSettings->Radius, ViewportClient->Viewport->KeyState(EdModeSettings->Add));
		break;
	}
	case EVoxelTool::Box:
	{
		SphereMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));
		ProjectionMarker->SetActorLocation(FVector(1e9, 1e9, 1e9));

		BoxMarker->SetParametersCpp(World, Mode, HitPosition, HitNormal, SpotlightHeight, ToolHeight, EdModeSettings->Radius, ViewportClient->Viewport->KeyState(EdModeSettings->Add));
		break;
	}
	default:
	{
		check(false);
		break;
	}
	}
	ViewportClient->Invalidate(false, false);

	return bMousePressed;
}

bool FVoxelTerrainEdMode::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	return MouseMove(ViewportClient, Viewport, MouseX, MouseY);
}

bool FVoxelTerrainEdMode::DisallowMouseDeltaTracking() const
{
	// We never want to use the mouse delta tracker while painting
	return bMousePressed;
}

void FVoxelTerrainEdMode::Tick(FEditorViewportClient * ViewportClient, float DeltaTime)
{
	if (!IsEnabled())
	{
		return;
	}
	{
		FActorSpawnParameters Transient;
		Transient.ObjectFlags = RF_Transient;

		if (!SphereMarker)
		{
			SphereMarker = GetWorld()->SpawnActor<AVoxelSphereMarker>(AVoxelSphereMarker::StaticClass(), Transient);
		}

		if (!BoxMarker)
		{
			BoxMarker = GetWorld()->SpawnActor<AVoxelBoxMarker>(AVoxelBoxMarker::StaticClass(), Transient);
		}

		if (!ProjectionMarker)
		{
			ProjectionMarker = GetWorld()->SpawnActor<AVoxelProjectionMarker>(AVoxelProjectionMarker::StaticClass(), Transient);
		}
	}

	ViewportClient->Viewport->CaptureMouse(true);
	Time += DeltaTime;

	if (Time - LastProjectionEditTime > 1 / 30 && bMousePressed && World)
	{
		LastProjectionEditTime = Time;
		if (Tool == EVoxelTool::Projection)
		{
			switch (Mode)
			{
			case EVoxelMode::Edit:
			{
				TArray<FIntVector> ModifiedPositions;
				UVoxelTools::SetValueProjection(ModifiedPositions, World, HitPosition, -HitNormal, EdModeSettings->Radius, EdModeSettings->Strength, ViewportClient->Viewport->KeyState(EdModeSettings->Add), ToolHeight);
				World->GetData()->SaveFrame();
				break;
			}
			case EVoxelMode::Flatten:
			{
				TArray<FIntVector> ModifiedPositions;
				UVoxelTools::Flatten(World, HitPosition, HitNormal, EdModeSettings->Radius, EdModeSettings->Strength);
				World->GetData()->SaveFrame();
				break;
			}
			case EVoxelMode::Material:
			{
				TArray<FIntVector> ModifiedPositions;
				UVoxelTools::SetMaterialProjection(World, HitPosition, -HitNormal, EdModeSettings->Radius, EdModeSettings->MaterialIndex, EdModeSettings->Layer, 3, 2, ToolHeight);
				World->GetData()->SaveFrame();
				break;
			}
			default:
			{
				check(false);
				break;
			}
			}
		}
	}

	if (Time - LastForceGCTime > 1)
	{
		LastForceGCTime = Time;
		GEngine->ForceGarbageCollection(true);
	}
}

bool FVoxelTerrainEdMode::HandleClick(FEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
{
	if (!IsEnabled())
	{
		return false;
	}

	if (!World)
	{
		return false;
	}

	switch (Tool)
	{
	case EVoxelTool::Projection:
	{
		break;
	}
	case EVoxelTool::Sphere:
	{
		if (Mode == EVoxelMode::Material)
		{
			UVoxelTools::SetMaterialSphere(World, HitPosition, EdModeSettings->Radius, EdModeSettings->MaterialIndex, EdModeSettings->Layer);
		}
		else
		{
			UVoxelTools::SetValueSphere(World, HitPosition, EdModeSettings->Radius, ViewportClient->Viewport->KeyState(EdModeSettings->Add));
		}
		World->GetData()->SaveFrame();
		break;
	}
	case EVoxelTool::Box:
	{
		const int R = FMath::CeilToInt(EdModeSettings->Radius / World->GetVoxelSize());
		const FVector P = HitPosition - EdModeSettings->Radius / 2;
		if (Mode == EVoxelMode::Material)
		{
			UVoxelTools::SetMaterialBox(World, P, FIntVector(R, R, R), EdModeSettings->MaterialIndex, EdModeSettings->Layer);
		}
		else
		{
			UVoxelTools::SetValueBox(World, P, FIntVector(R, R, R), ViewportClient->Viewport->KeyState(EdModeSettings->Add));
		}
		World->GetData()->SaveFrame();
		break;
	}
	default:
	{
		check(false);
		break;
	}
	}

	return true;
}

bool FVoxelTerrainEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (IsEnabled())
	{
		if (Event == IE_Released && (Key == EKeys::LeftMouseButton || Key == EKeys::MiddleMouseButton || Key == EKeys::RightMouseButton))
		{
			//Set the cursor position to that of the slate cursor so it wont snap back
			Viewport->SetPreCaptureMousePosFromSlateCursor();
		}
		if (Key == EKeys::LeftMouseButton && Event != EInputEvent::IE_Repeat)
		{
			bMousePressed = (Event == EInputEvent::IE_Pressed);
		}
	}

	return false;
}

bool FVoxelTerrainEdMode::InputDelta(FEditorViewportClient * InViewportClient, FViewport * InViewport, FVector & InDrag, FRotator & InRot, FVector & InScale)
{
	return false;
}

void FVoxelTerrainEdMode::Render(const FSceneView * View, FViewport * Viewport, FPrimitiveDrawInterface * PDI)
{

}

bool FVoxelTerrainEdMode::UsesToolkits() const
{
	return true;
}

bool FVoxelTerrainEdMode::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	return !IsEnabled(); // Disallow selection if enabled
}

bool FVoxelTerrainEdMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	if (IsEnabled())
	{
		OutCursor = EMouseCursor::Crosshairs;
		return true;
	}
	else
	{
		return false;
	}
}

void FVoxelTerrainEdMode::SetCurrentTool(FName InTool)
{
	CurrentToolName = InTool;
	Tool = CurrentToolName == "ProjectionTool" ? EVoxelTool::Projection : CurrentToolName == "SphereTool" ? EVoxelTool::Sphere : EVoxelTool::Box;
}

void FVoxelTerrainEdMode::SetCurrentMode(FName InMode)
{
	CurrentModeName = InMode;
	Mode = CurrentModeName == "EditMode" ? EVoxelMode::Edit : CurrentModeName == "PaintMode" ? EVoxelMode::Material : EVoxelMode::Flatten;
}

FName FVoxelTerrainEdMode::GetCurrentTool() const
{
	return CurrentToolName;
}

FName FVoxelTerrainEdMode::GetCurrentMode() const
{
	return CurrentModeName;
}

void FVoxelTerrainEdMode::Undo(int Times)
{
	if (World)
	{
		TArray<FIntVector> PositionsToUpdate;
		if (!World->GetData()->CheckIfCurrentFrameIsEmpty())
		{
			World->GetData()->SaveFrame();
		}

		for (int i = 0; i < Times; i++)
		{
			World->GetData()->Undo(PositionsToUpdate);
		}

		for (auto Position : PositionsToUpdate)
		{
			World->UpdateChunksAtPosition(Position);
		}
	}
}

void FVoxelTerrainEdMode::Redo(int Times)
{
	if (World)
	{
		TArray<FIntVector> PositionsToUpdate;
		if (!World->GetData()->CheckIfCurrentFrameIsEmpty())
		{
			World->GetData()->SaveFrame();
		}
		
		for (int i = 0; i < Times; i++)
		{
			World->GetData()->Redo(PositionsToUpdate);
		}
		
		for (auto Position : PositionsToUpdate)
		{
			World->UpdateChunksAtPosition(Position);
		}
	}
}

void FVoxelTerrainEdMode::ClearFrames()
{
	if (World)
	{
		World->GetData()->ClearFrames();
	}
}

bool FVoxelTerrainEdMode::IsEnabled() const
{
	return !GEditor->PlayWorld && !GEditor->bIsSimulatingInEditor;
}
