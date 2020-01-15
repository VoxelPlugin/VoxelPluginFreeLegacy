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

FVoxelToolManager_SurfaceSettings::FVoxelToolManager_SurfaceSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Surface"));
	static ConstructorHelpers::FObjectFinder<UTexture2D> MaskFinder(TEXT("/Voxel/Examples/VoxelDefaultBrushMask"));
	ToolMaterial = ToolMaterialFinder.Object;
	Mask = MaskFinder.Object;
}
FVoxelToolManager_TrimSettings::FVoxelToolManager_TrimSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Trim"));
	ToolMaterial = ToolMaterialFinder.Object;
}
FVoxelToolManager_SmoothSettings::FVoxelToolManager_SmoothSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Smooth"));
	ToolMaterial = ToolMaterialFinder.Object;
}
FVoxelToolManager_SphereSettings::FVoxelToolManager_SphereSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	ToolMaterial = ToolMaterialFinder.Object;
	SphereMesh = SphereMeshFinder.Object;
}
FVoxelToolManager_MeshSettings::FVoxelToolManager_MeshSettings()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Mesh"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ColorMaterialFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_Color"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UVsMaterialFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_UVs"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_SM_Chair"));
	ToolMaterial = ToolMaterialFinder.Object;
	ColorsMaterial = ColorMaterialFinder.Object;
	UVsMaterial = UVsMaterialFinder.Object;
	Mesh = MeshFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

	const FVector Start = TickData.RayOrigin;
	const FVector End = TickData.RayOrigin + WORLD_MAX * TickData.RayDirection;

	FHitResult HitResult;
	TickData.World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	AVoxelWorld* const NewVoxelWorld = Cast<AVoxelWorld>(HitResult.Actor);
	if (NewVoxelWorld && NewVoxelWorld->IsCreated())
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

	if (!VoxelWorld || !VoxelWorld->IsCreated())
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
	if (!ViewportClient->GetMousePosition(ScreenPosition))
	{
		// Too spammy FVoxelMessages::Warning(FUNCTION_ERROR("No mouse!"));
		return;
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

void UVoxelToolManager::Destroy()
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
	case EVoxelToolManagerTool::Trim: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Trim>(*this); break;
	case EVoxelToolManagerTool::Smooth: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Smooth>(*this); break;
	case EVoxelToolManagerTool::Sphere: ToolInstance = MakeVoxelShared<FVoxelToolManagerTool_Sphere>(*this); break;
	case EVoxelToolManagerTool::Mesh: FVoxelMessages::ShowVoxelPluginProError("Mesh Stamps are only available in the Pro version of Voxel Plugin"); break;
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
	
	if (Property.HasMetaData(STATIC_FNAME("ShowForMaterialConfigs")))
	{
		const FString& ShowForMaterialConfigs = Property.GetMetaData(STATIC_FNAME("ShowForMaterialConfigs"));
		if (VoxelWorld)
		{
			const FString MaterialConfig = GET_STATIC_UENUM(EVoxelMaterialConfig).GetNameStringByValue(int64(VoxelWorld->MaterialConfig));
			if (!ShowForMaterialConfigs.Contains(MaterialConfig))
			{
				return false;
			}
		}
	}

	const FString& ShowForTools = Property.GetMetaData(STATIC_FNAME("ShowForTools"));
	if (ShowForTools.Contains(STATIC_FSTRING("All")) ||
		ShowForTools.Contains(GET_STATIC_UENUM(EVoxelToolManagerTool).GetNameStringByValue(int64(Tool))))
	{
		return true;
	}
	if (ParentProperty)
	{
		if (IsPropertyVisible(*ParentProperty, nullptr))
		{
			if (Property.HasMetaData(STATIC_FNAME("OnlyIfNotSurfaceAlignment")))
			{
				const FString& OnlyIfNotSurfaceAlignment = Property.GetMetaData(STATIC_FNAME("OnlyIfNotSurfaceAlignment"));
				if (OnlyIfNotSurfaceAlignment == STATIC_FSTRING("Sphere"))
				{
					return SphereSettings.Alignment != EVoxelToolManagerAlignment::Surface;
				}
				if (OnlyIfNotSurfaceAlignment == STATIC_FSTRING("Mesh"))
				{
					return MeshSettings.Alignment != EVoxelToolManagerAlignment::Surface;
				}
				ensure(false);
				return true;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

void UVoxelToolManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && Tool == EVoxelToolManagerTool::Mesh)
	{
		// Recreate to avoid having to keep track of properties
		RecreateToolInstance();
	}
	
	const auto Name = PropertyChangedEvent.Property->GetFName();
	if (Name == GET_MEMBER_NAME_STATIC(UVoxelToolManager, Tool) ||
		Name == GET_MEMBER_NAME_STATIC(FVoxelToolManager_SphereSettings, Alignment))
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
		case EVoxelToolManagerTool::Surface: return SurfaceSettings.bPaint;
		case EVoxelToolManagerTool::Sphere: return SphereSettings.bPaint;
		case EVoxelToolManagerTool::Mesh:
		case EVoxelToolManagerTool::Trim:
		case EVoxelToolManagerTool::Smooth:
		default: ensure(false);
		}
	}
	if (Name == GET_MEMBER_NAME_STATIC(UVoxelToolManager, Radius))
	{
		switch (Tool)
		{
		case EVoxelToolManagerTool::Surface:
		case EVoxelToolManagerTool::Sphere:
		case EVoxelToolManagerTool::Trim:
		case EVoxelToolManagerTool::Smooth:
			return true;
		case EVoxelToolManagerTool::Mesh:
			return !MeshSettings.bUseMeshScale;
		default: ensure(false);
		}
	}

	return true;
}
#endif