// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelSmoothTool.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UVoxelSmoothTool::UVoxelSmoothTool()
{
	ToolName = TEXT("Smooth");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Smooth"));
	ToolMaterial = ToolMaterialFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSmoothTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.OverlayMaterial = ToolMaterial;
}

void UVoxelSmoothTool::Tick()
{
	Super::Tick();
	
	Falloff = GetValueAfterAxisInput(FVoxelToolAxes::Falloff, Falloff);
	Strength = GetValueAfterAxisInput(FVoxelToolAxes::Strength, Strength);
}

void UVoxelSmoothTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!OverlayMaterialInstance)
	{
		return;
	}

	const FVector Position = GetToolPreviewPosition();

	const float Radius = SharedConfig->BrushSize / 2.f;
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	OverlayMaterialInstance->SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), true);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FalloffType));

	SetToolOverlayBounds(FBox(Position - Radius, Position + Radius));
}

FVoxelIntBoxWithValidity UVoxelSmoothTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	auto& World = *GetVoxelWorld();

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = SharedConfig->BrushSize / 2.f / World.VoxelSize;

	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		return {};
	}
	const auto BoundsToCache = GetBoundsToCache(Bounds);

	auto& Data = World.GetData();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);
	CacheData<FVoxelValue>(Data, BoundsToCache);

	FVoxelSphereToolsImpl::SmoothSphere(
		DataImpl,
		VoxelPosition,
		VoxelRadius,
		Strength,
		GetTickData().IsAlternativeMode() ? 1 : NumIterations,
		FalloffType,
		Falloff);

	return Bounds;
}