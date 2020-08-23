// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelLevelTool.h"
#include "VoxelTools/Impl/VoxelLevelToolsImpl.inl"
#include "VoxelWorld.h"

#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelLevelTool::UVoxelLevelTool()
{
	ToolName = TEXT("Level");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Level"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder"));
	ToolMaterial = ToolMaterialFinder.Object;
	CylinderMesh = CylinderMeshFinder.Object;
}

void UVoxelLevelTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.MeshMaterial = ToolMaterial;

	OutConfig.Stride = Stride;
	OutConfig.bHasAlignment = true;
	OutConfig.Alignment = EVoxelToolAlignment::Ground;
	OutConfig.DistanceToCamera = 1e5;
}

void UVoxelLevelTool::Tick()
{
	Super::Tick();
	
	Falloff = GetValueAfterAxisInput(FVoxelToolAxes::Falloff, Falloff);
	Height = GetValueAfterAxisInput(FVoxelToolAxes::Strength, Height, 0, 10000);
}

void UVoxelLevelTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!MeshMaterialInstance)
	{
		return;
	}

	const bool bAlternativeMode = GetTickData().IsAlternativeMode();
	
	const float ScaleXY = SharedConfig->BrushSize / 100.f;
	const float ScaleZ = Height / 100.f + 0.001f;
	const FTransform PreviewTransform(
		FQuat::Identity,
		GetToolPreviewPosition() + GetToolOffset() + FVector(0, 0, (bAlternativeMode ? Height : -Height) / 2),
		FVector(ScaleXY, ScaleXY, ScaleZ));
	
	MeshMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);
	
	UpdateToolMesh(
		CylinderMesh,
		MeshMaterialInstance,
		PreviewTransform);
}

FVoxelIntBoxWithValidity UVoxelLevelTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	auto& World = *GetVoxelWorld();
	const bool bAlternativeMode = GetTickData().IsAlternativeMode();

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition() + GetToolOffset());
	const float VoxelRadius = SharedConfig->BrushSize / 2 / World.VoxelSize;
	const float VoxelHeight = Height / World.VoxelSize;

	const FVoxelIntBox Bounds = FVoxelLevelToolsImpl::GetBounds(VoxelPosition, VoxelRadius, VoxelHeight, !bAlternativeMode);
	if (!Bounds.IsValid())
	{
		return {};
	}
	const auto BoundsToCache = GetBoundsToCache(Bounds);

	auto& Data = World.GetData();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);
	CacheData<FVoxelValue>(Data, BoundsToCache);
	
	FVoxelLevelToolsImpl::Level(
		DataImpl,
		VoxelPosition,
		VoxelRadius,
		Falloff,
		VoxelHeight,
		!bAlternativeMode);

	return Bounds;
}

FVector UVoxelLevelTool::GetToolOffset() const
{
	return FVector(0.f, 0.f,  Offset * Height * (GetTickData().IsAlternativeMode() ? -1 : 1));
}