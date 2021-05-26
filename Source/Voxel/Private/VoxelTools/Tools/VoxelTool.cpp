// Copyright 2021 Phyronnaz

#include "VoxelTools/Tools/VoxelTool.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelWorld.h"

#include "Engine/StaticMesh.h"
#include "Engine/LocalPlayer.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

static bool GVoxelToolsAreFrozen = false;

static void FreezeVoxelTools()
{
	if (!GVoxelToolsAreFrozen)
	{
		GVoxelToolsAreFrozen = true;
		LOG_VOXEL(Log, TEXT("Freezing tool manager"));
	}
	else
	{
		GVoxelToolsAreFrozen = false;
		LOG_VOXEL(Log, TEXT("Unfreezing tool manager"));
	}
}

static FAutoConsoleCommand CmdFreeze(
    TEXT("voxel.tools.Freeze"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&FreezeVoxelTools));

///////////////////////////////////////////////////////////////////////////////

UVoxelToolSharedConfig::UVoxelToolSharedConfig()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaneMaterialFinder(TEXT("/Voxel/ToolMaterials/ViewportPlaneMaterial"));

	PlaneMesh = PlaneMeshFinder.Object;
	PlaneMaterial = PlaneMaterialFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::EnableTool()
{
	ensure(!bEnabled);
	bEnabled = true;

	if (!SharedConfig)
	{
		SharedConfig = NewObject<UVoxelToolSharedConfig>(this);
	}
}

void UVoxelTool::DisableTool()
{
	ensure(bEnabled);
	bEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::K2_AdvancedTick(UObject* WorldContextObject, const FVoxelToolTickData& TickData, const FDoEditDynamicOverride& DoEditOverride)
{
	if (!WorldContextObject)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid WorldContextObject!"));
		return;
	}
	
	FDoEditOverride DoEditOverrideCpp;
	if (DoEditOverride.IsBound())
	{
		DoEditOverrideCpp.BindLambda([&](FVector Position, FVector Normal) { DoEditOverride.Execute(Position, Normal); });
	}
	AdvancedTick(WorldContextObject->GetWorld(), TickData, DoEditOverrideCpp);
}

void UVoxelTool::AdvancedTick(UWorld* World, const FVoxelToolTickData& TickData, const FDoEditOverride& DoEditOverride)
{
	VOXEL_FUNCTION_COUNTER();

	if (!bEnabled)
	{
		EnableTool();
	}

	if (!World)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid World!"));
		return;
	}
	
	const auto CanEditWorld = [&](AVoxelWorld* InWorld)
	{
		return InWorld && InWorld->IsCreated() && (SharedConfig->WorldsToEdit.Num() == 0 || SharedConfig->WorldsToEdit.Contains(InWorld));
	};

	const FVector Start = TickData.GetRayOrigin();
	const FVector End = TickData.GetRayOrigin() + float(WORLD_MAX) * TickData.GetRayDirection();

	FHitResult HitResult;
	World->GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, TickData.CollisionChannel);
	
	AVoxelWorld* VoxelWorld = Cast<AVoxelWorld>(HitResult.UE_5_SWITCH(Actor, GetActor()));
	if (!CanEditWorld(VoxelWorld))
	{
		VoxelWorld = nullptr;
	}

#if WITH_EDITOR
	if (VoxelWorld && SharedConfig->PaintMaterial.PreviewVoxelWorld != VoxelWorld)
	{
		SharedConfig->PaintMaterial.PreviewVoxelWorld = VoxelWorld;
		SharedConfig->RefreshDetails.Broadcast();
	}
#endif
	
	if (!GVoxelToolsAreFrozen)
	{
		FrozenTickData = TickData;
	}

	FCallToolParameters Parameters;
	Parameters.Mode = ECallToolMode::Tick;
	Parameters.Position = HitResult.ImpactPoint;
	Parameters.Normal = HitResult.ImpactNormal;
	Parameters.bBlockingHit = HitResult.bBlockingHit;

	if (DoEditOverride.IsBound())
	{
		Parameters.DoEditOverride = [&](FVector Position, FVector Normal) { DoEditOverride.Execute(Position, Normal); };
	}
	
	CallTool(VoxelWorld, GVoxelToolsAreFrozen ? FrozenTickData : TickData, Parameters);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::K2_SimpleTick(
	APlayerController* PlayerController,
	bool bEdit,
	const TMap<FName, bool>& Keys,
	const TMap<FName, float>& Axes,
	const FDoEditDynamicOverride& DoEditOverride,
	ECollisionChannel CollisionChannel)
{
	FDoEditOverride DoEditOverrideCpp;
	if (DoEditOverride.IsBound())
	{
		DoEditOverrideCpp.BindLambda([&](FVector Position, FVector Normal) { DoEditOverride.Execute(Position, Normal); });
	}
	SimpleTick(PlayerController, bEdit, Keys, Axes, DoEditOverrideCpp, CollisionChannel);
}

void UVoxelTool::SimpleTick(
	APlayerController* PlayerController,
	bool bEdit,
	const TMap<FName, bool>& Keys,
	const TMap<FName, float>& Axes,
	const FDoEditOverride& DoEditOverride,
	ECollisionChannel CollisionChannel)
{
	VOXEL_FUNCTION_COUNTER();

	if (!PlayerController)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid PlayerController!"));
		return;
	}

	ULocalPlayer* const LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid LocalPlayer!"));
		return;
	}

	auto ViewportClient = LocalPlayer->ViewportClient;
	if (!ViewportClient)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid ViewportClient!"));
		return;
	}

	FVector2D ScreenPosition;
	if (!ViewportClient->GetMousePosition(ScreenPosition))
	{
		// This happen when the mouse is over the Unreal UI: use the center
		FVector2D Size;
		ViewportClient->GetViewportSize(Size);
		ScreenPosition = Size / 2;

		// Make sure to do nothing when clicking on the UI
		bEdit = false;
	}
	
	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
	if (!PlayerCameraManager)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid PlayerCameraManager!"));
		return;
	}

	const auto TickData = MakeVoxelToolTickData(
		PlayerController,
		bEdit,
		Keys,
		Axes,
		ScreenPosition,
		PlayerCameraManager->GetCameraRotation().Vector(),
		CollisionChannel);

	AdvancedTick(PlayerController->GetWorld(), TickData, DoEditOverride);
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::Apply(
	AVoxelWorld* World, 
	FVector Position, 
	FVector Normal, 
	const TMap<FName, bool>& Keys, 
	const TMap<FName, float>& Axes)
{
	CHECK_VOXELWORLD_IS_CREATED_VOID();

	if (!bEnabled)
	{
		EnableTool();
	}

	Normal = Normal.GetSafeNormal();
	if (Normal.IsNearlyZero())
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Normal is zero, using UpVector instead"));
		Normal = FVector::UpVector;
	}

	FVoxelToolTickData TickData;
	TickData.bEdit = true;
	TickData.Keys = Keys;
	TickData.Axes = Axes;
	
	FCallToolParameters Parameters;
	Parameters.Mode = ECallToolMode::Apply;
	Parameters.Position = Position;
	Parameters.Normal = Normal;
	Parameters.bBlockingHit = true;
	
	CallTool(World, TickData, Parameters);
}

///////////////////////////////////////////////////////////////////////////////

FName UVoxelTool::GetToolName() const
{
	return ToolName.IsNone() ? GetClass()->GetFName() : ToolName;
}

///////////////////////////////////////////////////////////////////////////////

TMap<FName, bool> UVoxelTool::MakeToolKeys(bool bAlternativeMode)
{
	return { { FVoxelToolKeys::AlternativeMode, bAlternativeMode } };
}

TMap<FName, float> UVoxelTool::MakeToolAxes(float BrushSizeDelta, float FalloffDelta, float StrengthDelta)
{
	return
	{
		{
			FVoxelToolAxes::BrushSize,
			BrushSizeDelta
		},
		{
			FVoxelToolAxes::Falloff,
			FalloffDelta
		},
		{
			FVoxelToolAxes::Strength,
			StrengthDelta
		}
	};
}

UVoxelTool* UVoxelTool::MakeVoxelTool(TSubclassOf<UVoxelTool> ToolClass)
{
	if (!ToolClass)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("null ToolClass"));
		return nullptr;
	}
	if (ToolClass->HasAllClassFlags(CLASS_Abstract))
	{
		FVoxelMessages::Error(FUNCTION_ERROR("ToolClass is abstract"));
		return nullptr;
	}

	auto* Tool = NewObject<UVoxelTool>(GetTransientPackage(), ToolClass);
	if (!ensure(Tool))
	{
		return nullptr;
	}

	Tool->SharedConfig = NewObject<UVoxelToolSharedConfig>(Tool);
	return Tool;
}

FVoxelToolTickData UVoxelTool::MakeVoxelToolTickData(
	APlayerController* PlayerController, 
	bool bEdit, 
	const TMap<FName, bool>& Keys, 
	const TMap<FName, float>& Axes, 
	FVector2D MousePosition, 
	FVector CameraDirection,
	ECollisionChannel CollisionChannel)
{
	if (!PlayerController)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("PlayerController is null"));
		return {};
	}

	FVoxelToolTickData TickData;
	TickData.MousePosition = MousePosition;
	TickData.CameraViewDirection = CameraDirection;
	TickData.bEdit = bEdit;
	TickData.Keys = Keys;
	TickData.Axes = Axes;
	TickData.CollisionChannel = CollisionChannel;

	const auto Deproject = [PlayerController = MakeWeakObjectPtr(PlayerController)](
		const FVector2D& InScreenPosition,
		FVector& OutWorldPosition,
		FVector& OutWorldDirection)
	{
		return UGameplayStatics::DeprojectScreenToWorld(PlayerController.Get(), InScreenPosition, OutWorldPosition, OutWorldDirection);
	};
	TickData.Init(Deproject);

	return TickData;
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelTool::BeginDestroy()
{
	// Make sure to not leave any mesh behind
	if (bEnabled)
	{
		DisableTool();
	}
	
	Super::BeginDestroy();
}