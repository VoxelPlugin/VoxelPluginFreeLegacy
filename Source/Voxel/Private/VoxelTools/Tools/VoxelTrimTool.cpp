// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelTrimTool.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"
#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelTrimTool::UVoxelTrimTool()
{
	ToolName = TEXT("Trim");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Trim"));
	ToolMaterial = ToolMaterialFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelTrimTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.OverlayMaterial = ToolMaterial;
}

void UVoxelTrimTool::Tick()
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::Tick();
	
	FVoxelLineTraceParameters Parameters;
	Parameters.CollisionChannel = GetTickData().CollisionChannel;
	Parameters.DrawDebugType = SharedConfig->bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	TArray<FVoxelProjectionHit> Hits;
	const float RaysRadius = FMath::Max(SharedConfig->BrushSize / 2 * (1 - Roughness), GetVoxelWorld()->VoxelSize);
	const FVector RayDirection = GetTickData().GetRayDirection();
	
	UVoxelProjectionTools::FindProjectionVoxels(
		Hits,
		GetVoxelWorld(),
		Parameters,
		GetToolPosition() - RayDirection * RaysRadius,
		RayDirection,
		RaysRadius,
		EVoxelProjectionShape::Circle,
		100.f,
		2 * RaysRadius);

	Position = UVoxelProjectionTools::GetHitsAveragePosition(Hits);
	Normal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
}

void UVoxelTrimTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!OverlayMaterialInstance)
	{
		return;
	}

	const float Radius = SharedConfig->BrushSize / 2;
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	OverlayMaterialInstance->SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Roughness"), Roughness);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);

	SetToolOverlayBounds(FBox(Position - Radius, Position + Radius));
}

FVoxelIntBoxWithValidity UVoxelTrimTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	auto& World = *GetVoxelWorld();
	
	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(Position);
	const float VoxelRadius = SharedConfig->BrushSize / 2 / World.VoxelSize;

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
		
	FVoxelSphereToolsImpl::TrimSphere(
		DataImpl,
		VoxelPosition,
		Normal,
		VoxelRadius,
		Falloff,
		!GetTickData().IsAlternativeMode());

	return Bounds;
}