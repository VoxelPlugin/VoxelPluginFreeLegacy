// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Flatten.h"
#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerToolsHelpers.h"

#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "VoxelData/VoxelData.h"

#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings FlattenToolSettings(const FVoxelToolManager_FlattenSettings& FlattenSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Flatten Tool");
	
	Settings.Ptr_ToolOverlayMaterial = &FlattenSettings.ToolMaterial;
	
	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Flatten::FVoxelToolManagerTool_Flatten(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, FlattenToolSettings(ToolManager.FlattenSettings))
	, FlattenSettings(ToolManager.FlattenSettings)
{
}

void FVoxelToolManagerTool_Flatten::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	auto& MaterialInstance = GetToolOverlayMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), FlattenSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), FlattenSettings.bEnableFalloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FlattenSettings.FalloffType));
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), FlattenSettings.ToolOpacity);

	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	if (!CanEdit())
	{
		return;
	}

	constexpr float DistanceDivisor = 4.f;

	const FVoxelIntBox Bounds = UVoxelBlueprintLibrary::MakeIntBoxFromGlobalPositionAndRadius(&World, GetToolPosition(), ToolManager.Radius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	FVector FlattenPosition;
	FVector FlattenNormal;
	if (FlattenSettings.bUseAverage)
	{
		FVoxelLineTraceParameters Parameters;
		Parameters.CollisionChannel = World.CollisionPresets.GetObjectType();
		Parameters.DrawDebugType = ToolManager.bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

		const float RaysRadius = FMath::Max(ToolManager.Radius, World.VoxelSize);

		TArray<FVoxelProjectionHit> Hits;
		UVoxelProjectionTools::FindProjectionVoxels(
			Hits,
			&World,
			Parameters,
			GetToolPosition() - TickData.RayDirection * ToolManager.Radius,
			TickData.RayDirection,
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

	if (!GetLastFrameTickData().bClick)
	{
		LastClickFlattenPosition = FlattenPosition;
		LastClickFlattenNormal = FlattenNormal;
	}

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);
		
		const FVoxelSurfaceEditsVoxels Voxels = UVoxelSurfaceTools::FindSurfaceVoxelsFromDistanceFieldImpl(Data, Bounds, FlattenSettings.Strength + 3, true);

		FVoxelSurfaceEditsStack Stack;

		if (FlattenSettings.bEnableFalloff)
		{
			Stack.Add(FVoxelToolManagerToolsHelpers::GetApplyFalloff(
				FlattenSettings.FalloffType,
				World,
				GetToolPosition(),
				ToolManager.Radius,
				FlattenSettings.Falloff));
		}

		Stack.Add(UVoxelSurfaceTools::ApplyConstantStrength(FlattenSettings.Strength));

		const FVector PlanePoint = FlattenSettings.bFreezeOnClick ? LastClickFlattenPosition : FlattenPosition;
		const FVector PlaneNormal = FlattenSettings.bFreezeOnClick ? LastClickFlattenNormal : FlattenNormal;

		Stack.Add(UVoxelSurfaceTools::ApplyFlatten(
			&World,
			PlanePoint,
			PlaneNormal,
			TickData.bAlternativeMode ? EVoxelSDFMergeMode::Intersection : EVoxelSDFMergeMode::Union));

		const auto ProcessedVoxels = Stack.Execute(Voxels, false);
		
		UVoxelSurfaceTools::EditVoxelValuesImpl(Data, Bounds, ProcessedVoxels, FVoxelHardnessHandler(World), DistanceDivisor);

		if (ToolManager.bDebug)
		{
			UVoxelSurfaceTools::DebugSurfaceVoxels(&World, ProcessedVoxels, 2 * GetDeltaTime());

			DrawDebugSolidPlane(
				TickData.World,
				FPlane(PlanePoint, PlaneNormal),
				PlanePoint,
				1000000,
				FColor::Red,
				false,
				1.5f * GetDeltaTime());
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}