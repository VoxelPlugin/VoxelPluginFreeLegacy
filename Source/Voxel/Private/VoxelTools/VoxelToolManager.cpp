// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManager.h"
#include "VoxelTools/VoxelToolManagerTools.h"
#include "VoxelData/VoxelData.h"
#include "VoxelMessages.h"
#include "VoxelWorld.h"

#include "UObject/ConstructorHelpers.h"
#include "UObject/PropertyPortFlags.h"
#include "Misc/ConfigCacheIni.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

static bool GToolManagerIsFrozen = false;

static void FreezeToolManager()
{
	if (!GToolManagerIsFrozen)
	{
		GToolManagerIsFrozen = true;
		LOG_VOXEL(Log, TEXT("Freezing tool manager"));
	}
	else
	{
		GToolManagerIsFrozen = false;
		LOG_VOXEL(Log, TEXT("Unfreezing tool manager"));
	}
}

static FAutoConsoleCommand CmdFreeze(
    TEXT("voxel.ToolManager.Freeze"),
    TEXT(""),
    FConsoleCommandDelegate::CreateStatic(&FreezeToolManager));

FVoxelToolManager_SurfaceSettings::FVoxelToolManager_SurfaceSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Surface"));
	ToolMaterial = ToolMaterialFinder.Object;
	MaskTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Voxel/Examples/VoxelDefaultBrushMask"));
}

FVoxelToolManager_FlattenSettings::FVoxelToolManager_FlattenSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Flatten"));
	ToolMaterial = ToolMaterialFinder.Object;
}

FVoxelToolManager_TrimSettings::FVoxelToolManager_TrimSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Trim"));
	ToolMaterial = ToolMaterialFinder.Object;
}

FVoxelToolManager_LevelSettings::FVoxelToolManager_LevelSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Level"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder"));
	ToolMaterial = ToolMaterialFinder.Object;
	CylinderMesh = CylinderMeshFinder.Object;
}

FVoxelToolManager_SmoothSettings::FVoxelToolManager_SmoothSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Smooth"));
	ToolMaterial = ToolMaterialFinder.Object;
}
FVoxelToolManager_SphereSettingsBase::FVoxelToolManager_SphereSettingsBase()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	ToolMaterial = ToolMaterialFinder.Object;
	SphereMesh = SphereMeshFinder.Object;
}
FVoxelToolManager_MeshSettings::FVoxelToolManager_MeshSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Mesh"));
	ToolMaterial = ToolMaterialFinder.Object;
	ColorsMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_Color"));
	UVsMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_UVs"));
	Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_SM_Chair"));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelToolManager::UVoxelToolManager()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlaneMaterialFinder(TEXT("/Voxel/ToolMaterials/ViewportPlaneMaterial"));

	PlaneMesh = PlaneMeshFinder.Object;
	PlaneMaterial = PlaneMaterialFinder.Object;
}

void UVoxelToolManager::SaveConfig()
{
	const UVoxelToolManager* CDO = GetDefault<UVoxelToolManager>();

	for (TFieldIterator<UProperty> It(UVoxelToolManager::StaticClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		auto& Property = **It;
		if (Property.HasAnyPropertyFlags(CPF_Transient)) continue;
		if (!ensure(Property.ArrayDim == 1)) continue;

		const TCHAR* const Section = TEXT("VoxelToolManager");

		FString	Value;
		if (Property.ExportText_InContainer(0, Value, this, CDO, this, PPF_None))
		{
			GConfig->SetString(Section, *Property.GetName(), *Value, GEditorPerProjectIni);
		}
		else
		{
			GConfig->RemoveKey(Section, *Property.GetName(), GEditorPerProjectIni);
		}
	}
}

void UVoxelToolManager::LoadConfig()
{
	if (!SurfaceSettings.MaskWorldGenerator.IsValid())
	{
		// Hack needed as voxel graph module is not yet loaded when the constructor is called
		SurfaceSettings.MaskWorldGenerator.Type = EVoxelWorldGeneratorPickerType::Object;
		SurfaceSettings.MaskWorldGenerator.Object = LoadObject<UVoxelWorldGenerator>(nullptr, TEXT("/Voxel/Examples/VoxelGraphs/VoxelGraph_Mask_Cellular"));
	}
	for (TFieldIterator<UProperty> It(UVoxelToolManager::StaticClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		auto& Property = **It;
		if (Property.HasAnyPropertyFlags(CPF_Transient)) continue;
		if (!ensure(Property.ArrayDim == 1)) continue;

		const TCHAR* const Section = TEXT("VoxelToolManager");

		FString Value;
		if (GConfig->GetString(Section, *Property.GetName(), Value, GEditorPerProjectIni))
		{
			Property.ImportText(*Value, Property.ContainerPtrToValuePtr<void>(this), PPF_None, this);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelToolManager::Tick(FVoxelToolManagerTickData TickData)
{
	VOXEL_FUNCTION_COUNTER();

	static FVoxelToolManagerTickData FrozenTickData;
	if (GToolManagerIsFrozen)
	{
		TickData = FrozenTickData;
	}
	else
	{
		FrozenTickData = TickData;
	}

	if (!TickData.World)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid TickData.World!"));
		return;
	}
	if (!TickData.CameraViewDirection.Normalize())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid TickData.CameraViewDirection!"));
		return;
	}
	if (!TickData.RayDirection.Normalize())
	{
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid TickData.RayDirection!"));
		return;
	}
	
	Radius = FMath::Max(0.f, Radius + TickData.MouseWheelDelta * RadiusChangeSpeed);

	const auto CanEditWorld = [&](AVoxelWorld* InWorld)
	{
		return InWorld && InWorld->IsCreated() && (WorldsToEdit.Num() == 0 || WorldsToEdit.Contains(InWorld));
	};

	const FVector Start = TickData.RayOrigin;
	const FVector End = TickData.RayOrigin + float(WORLD_MAX) * TickData.RayDirection;

	FHitResult HitResult;
	TickData.World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	AVoxelWorld* const NewVoxelWorld = Cast<AVoxelWorld>(HitResult.Actor);
	if (CanEditWorld(NewVoxelWorld))
	{
		VoxelWorld = NewVoxelWorld;
	}

	if (ToolInstanceType != Tool)
	{
		ToolInstance.Reset();
	}
	if (!ToolInstance.IsValid())
	{
		RecreateToolInstance();
	}
	if (!ToolInstance.IsValid()) return;

	if (!CanEditWorld(VoxelWorld))
	{
		ToolInstance->ClearVoxelWorld();
		return;
	}

#if WITH_EDITOR
	PaintMaterial.bRestrictType = true;
	if (PaintMaterial.MaterialConfigToRestrictTo != VoxelWorld->MaterialConfig)
	{
		PaintMaterial.MaterialConfigToRestrictTo = VoxelWorld->MaterialConfig;
		RefreshDetails.Broadcast();
	}
#endif

	ToolInstance->TriggerTick(*VoxelWorld, TickData, HitResult);
}

void UVoxelToolManager::SimpleTick(APlayerController* PlayerController, bool bClick, bool bAlternativeMode, float MouseWheelDelta)
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

	auto* ViewportClient = LocalPlayer->ViewportClient;
	if (!ViewportClient)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid ViewportClient!"));
		return;
	}

	FVector2D ScreenPosition;
	if (ViewportClient->GetMousePosition(ScreenPosition))
	{
		SimpleTick_LastKnownMousePosition = ScreenPosition;
	}
	else
	{
		ScreenPosition = SimpleTick_LastKnownMousePosition;
		bClick = false; // Make sure to do nothing when clicking on the UI
	}
	
	FVector WorldPosition;
	FVector WorldDirection;
	if (!UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPosition, WorldPosition, WorldDirection))
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid viewport!"));
		return;
	}
	
	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
	if (!PlayerCameraManager)
	{
		FVoxelMessages::Warning(FUNCTION_ERROR("Invalid PlayerCameraManager!"));
		return;
	}

	FVoxelToolManagerTickData TickData;
	TickData.World = PlayerController->GetWorld();
	TickData.DeltaTime = TickData.World->GetDeltaSeconds();
	TickData.MousePosition = ScreenPosition;
	TickData.CameraViewDirection = PlayerCameraManager->GetCameraRotation().Vector();
	TickData.RayOrigin = WorldPosition;
	TickData.RayDirection = WorldDirection;
	TickData.bClick = bClick;
	TickData.bAlternativeMode = bAlternativeMode;
	TickData.MouseWheelDelta = MouseWheelDelta;

	Tick(TickData);
}

void UVoxelToolManager::ClearToolInstance()
{
	VOXEL_FUNCTION_COUNTER();

	if (ToolInstance.IsValid())
	{
		ToolInstance->ClearVoxelWorld();
		ToolInstance.Reset();
	}
}

void UVoxelToolManager::RecreateToolInstance()
{
	ToolInstance.Reset();
	switch (Tool)
	{
	case EVoxelToolManagerTool::Surface: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Surface>(*this); break;
	case EVoxelToolManagerTool::Flatten: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Flatten>(*this); break;
	case EVoxelToolManagerTool::Trim: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Trim>(*this); break;
	case EVoxelToolManagerTool::Level: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Level>(*this); break;
	case EVoxelToolManagerTool::Smooth: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Smooth>(*this); break;
	case EVoxelToolManagerTool::Sphere: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Sphere>(*this); break;
	case EVoxelToolManagerTool::Mesh: FVoxelMessages::ShowVoxelPluginProError("Mesh Stamps are only available in the Pro version of Voxel Plugin"); break;
	case EVoxelToolManagerTool::Revert: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Revert>(*this); break;
	case EVoxelToolManagerTool::Custom: break; 
	default: ensure(false);
	}
	ToolInstanceType = Tool;
}

void UVoxelToolManager::SaveFrame(AVoxelWorld& World, const FIntBox& Bounds, FName Name) const
{
	auto& Data = World.GetData();
	if (Data.bEnableUndoRedo)
	{
		Data.SaveFrame(Bounds);
		RegisterTransaction.Broadcast(Name, &World);
	}
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
bool UVoxelToolManager::IsPropertyVisible(const UProperty& Property, const UProperty* ParentProperty) const
{
	if (Property.HasMetaData(STATIC_FNAME("HideInPanel")))
	{
		return false;
	}

	if (Property.HasMetaData(STATIC_FNAME("ShowForTools")))
	{
		// If we have a ShowForTools meta and it matches the tool, we're visible
		const FString& ShowForTools = Property.GetMetaData(STATIC_FNAME("ShowForTools"));
		if (ShowForTools.Contains(STATIC_FSTRING("All")) ||
			ShowForTools.Contains(StaticEnum<EVoxelToolManagerTool>()->GetNameStringByValue(int64(Tool))))
		{
			return true;
		}
	}

	// Else check the parent
	if (!ParentProperty)
	{
		return false;
	}
	if (!IsPropertyVisible(*ParentProperty, nullptr))
	{
		return false;
	}

	if (Property.HasMetaData(STATIC_FNAME("ShowForMaterialConfigs")))
	{
		const FString& ShowForMaterialConfigs = Property.GetMetaData(STATIC_FNAME("ShowForMaterialConfigs"));
		if (VoxelWorld)
		{
			const FString MaterialConfig = StaticEnum<EVoxelMaterialConfig>()->GetNameStringByValue(int64(VoxelWorld->MaterialConfig));
			if (!ShowForMaterialConfigs.Contains(MaterialConfig))
			{
				return false;
			}
		}
	}

	if (Property.HasMetaData(STATIC_FNAME("OnlyIfNotSurfaceAlignment")))
	{
		const auto GetAlignment = [&]()
		{
			switch (Tool)
			{
			default: ensure(false);
			case EVoxelToolManagerTool::Sphere:
				return SphereSettings.Alignment;
			case EVoxelToolManagerTool::Mesh:
				return MeshSettings.Alignment;
			case EVoxelToolManagerTool::Revert:
				return RevertSettings.Alignment;
			}
		};
		return GetAlignment() != EVoxelToolManagerAlignment::Surface;
	}
	
	if (Property.HasMetaData(STATIC_FNAME("OnlyIfMaskType")))
	{
		const FString& OnlyIfMaskType = Property.GetMetaData(STATIC_FNAME("OnlyIfMaskType"));
		if (OnlyIfMaskType == STATIC_FSTRING("Texture"))
		{
			return SurfaceSettings.MaskType == EVoxelToolManagerMaskType::Texture;
		}
		if (OnlyIfMaskType == STATIC_FSTRING("WorldGenerator"))
		{
			return SurfaceSettings.MaskType == EVoxelToolManagerMaskType::WorldGenerator;
		}
		ensure(false);
	}

	return true;
}

void UVoxelToolManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && Tool == EVoxelToolManagerTool::Mesh)
	{
		// Recreate to avoid having to keep track of properties
		RecreateToolInstance();
	}

	if (PropertyChangedEvent.Property)
	{
		const auto Name = PropertyChangedEvent.Property->GetFName();
		if (Name == GET_MEMBER_NAME_STATIC(UVoxelToolManager, Tool) ||
			Name == GET_MEMBER_NAME_STATIC(FVoxelToolManager_SurfaceSettings, MaskType) ||
			Name == GET_MEMBER_NAME_STATIC(FVoxelToolManager_SphereSettings, Alignment))
		{
			RefreshDetails.Broadcast();
		}
	}
	else
	{
		RefreshDetails.Broadcast();
	}
}

bool UVoxelToolManager::CanEditChange(const UProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty)) return false;

	const auto Name = InProperty->GetFName();
	if (Name == GET_MEMBER_NAME_STATIC(UVoxelToolManager, PaintMaterial))
	{
		switch (Tool)
		{
		case EVoxelToolManagerTool::Surface:
			return SurfaceSettings.bPaint;
		case EVoxelToolManagerTool::Sphere:
			return SphereSettings.bPaint;
		default:
			return true;
		}
	}
	if (Name == GET_MEMBER_NAME_STATIC(UVoxelToolManager, Radius))
	{
		switch (Tool)
		{
		case EVoxelToolManagerTool::Mesh:
			return !MeshSettings.bUseMeshScale;
		default:
			return true;
		}
	}

	return true;
}
#endif