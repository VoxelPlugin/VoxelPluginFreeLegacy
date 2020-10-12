// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelFlattenTool.h"

#include "VoxelWorld.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.inl"
#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelTools/VoxelHardnessHandler.h"

#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelFlattenTool::UVoxelFlattenTool()
{
	ToolName = TEXT("Flatten");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Flatten"));
	ToolMaterial = ToolMaterialFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelFlattenTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.OverlayMaterial = ToolMaterial;
}

void UVoxelFlattenTool::Tick()
{
	Super::Tick();
	
	Falloff = GetValueAfterAxisInput(FVoxelToolAxes::Falloff, Falloff);
	Strength = GetValueAfterAxisInput(FVoxelToolAxes::Strength, Strength);
}

void UVoxelFlattenTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!OverlayMaterialInstance)
	{
		return;
	}

	const float Radius = SharedConfig->BrushSize / 2;
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	OverlayMaterialInstance->SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), bEnableFalloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FalloffType));
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);

	SetToolOverlayBounds(FBox(GetToolPreviewPosition() - Radius, GetToolPreviewPosition() + Radius));
}

FVoxelIntBoxWithValidity UVoxelFlattenTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	constexpr float DistanceDivisor = 4.f;
	const float Radius = SharedConfig->BrushSize / 2;
	
	const FVoxelIntBox Bounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(GetVoxelWorld(), GetToolPosition(), Radius);
	if (!Bounds.IsValid())
	{
		return {};
	}
	const auto BoundsToCache = GetBoundsToCache(Bounds);

	FVector FlattenPosition;
	FVector FlattenNormal;
	if (bUseAverage)
	{
		FVoxelLineTraceParameters Parameters;
		Parameters.CollisionChannel = GetTickData().CollisionChannel;
		Parameters.DrawDebugType = SharedConfig->bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

		const float RaysRadius = FMath::Max(Radius, GetVoxelWorld()->VoxelSize);
		const FVector RayDirection = GetTickData().GetRayDirection();
		
		TArray<FVoxelProjectionHit> Hits;
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

		FlattenPosition = UVoxelProjectionTools::GetHitsAveragePosition(Hits);
		FlattenNormal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
	}
	else
	{
		FlattenPosition = GetToolPosition();
		FlattenNormal = GetToolNormal();
	}

	if (bUseFixedRotation)
	{
		FlattenNormal = FixedRotation.RotateVector(FVector::UpVector).GetSafeNormal();
	}

	if (!GetLastFrameTickData().bEdit)
	{
		LastClickFlattenPosition = FlattenPosition;
		LastClickFlattenNormal = FlattenNormal;
	}

	auto& Data = GetVoxelWorld()->GetData();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);
	CacheData<FVoxelValue>(Data, BoundsToCache);
	
	const FVoxelSurfaceEditsVoxels Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, SharedConfig->bMultiThreaded, SharedConfig->GetComputeDevice());

	FVoxelSurfaceEditsStack Stack;

	if (bEnableFalloff)
	{
		Stack.Add(UVoxelSurfaceTools::ApplyFalloff(
			GetVoxelWorld(),
			FalloffType,
			GetToolPosition(),
			Radius,
			Falloff));
	}

	Stack.Add(UVoxelSurfaceTools::ApplyConstantStrength(Strength));

	const FVector PlanePoint = bFreezeOnClick ? LastClickFlattenPosition : FlattenPosition;
	const FVector PlaneNormal = bFreezeOnClick ? LastClickFlattenNormal : FlattenNormal;

	Stack.Add(UVoxelSurfaceTools::ApplyFlatten(
		GetVoxelWorld(),
		PlanePoint,
		PlaneNormal,
		GetTickData().IsAlternativeMode() ? EVoxelSDFMergeMode::Intersection : EVoxelSDFMergeMode::Union));

	const auto ProcessedVoxels = Stack.Execute(Voxels, false);

	if (bPropagateMaterials)
	{
		FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(Data, ProcessedVoxels);
	}
	
	FVoxelSurfaceEditToolsImpl::EditVoxelValues(DataImpl, FVoxelHardnessHandler(*GetVoxelWorld()), Bounds, ProcessedVoxels, DistanceDivisor);

	if (SharedConfig->bDebug)
	{
		UVoxelSurfaceTools::DebugSurfaceVoxels(GetVoxelWorld(), ProcessedVoxels, 2 * GetDeltaTime());

		DrawDebugSolidPlane(
			GetVoxelWorld()->GetWorld(),
			FPlane(PlanePoint, PlaneNormal),
			PlanePoint,
			1000000,
			FColor::Red,
			false,
			1.5f * GetDeltaTime());
	}

	return Bounds;
}