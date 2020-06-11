// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Trim.h"

#include "VoxelTools/VoxelProjectionTools.h"
#include "VoxelTools/VoxelSphereTools.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings TrimToolSettings(const FVoxelToolManager_TrimSettings& TrimSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Trim Tool");

	Settings.Ptr_ToolOverlayMaterial = &TrimSettings.ToolMaterial;

	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Trim::FVoxelToolManagerTool_Trim(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, TrimToolSettings(ToolManager.TrimSettings))
	, TrimSettings(ToolManager.TrimSettings)
{
}

void FVoxelToolManagerTool_Trim::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelLineTraceParameters Parameters;
	Parameters.CollisionChannel = World.CollisionPresets.GetObjectType();
	Parameters.DrawDebugType = ToolManager.bDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	TArray<FVoxelProjectionHit> Hits;
	const float RaysRadius = FMath::Max(ToolManager.Radius * (1 - TrimSettings.Roughness), World.VoxelSize);
	UVoxelProjectionTools::FindProjectionVoxels(
		Hits,
		&World,
		Parameters,
		GetToolPosition() - TickData.RayDirection * RaysRadius,
		TickData.RayDirection,
		RaysRadius,
		EVoxelProjectionShape::Circle,
		100.f,
		2 * RaysRadius);

	const FVector Position = UVoxelProjectionTools::GetHitsAveragePosition(Hits);

	SetToolRenderingBounds(World, FBox(Position - ToolManager.Radius, Position + ToolManager.Radius));

	auto& MaterialInstance = GetToolOverlayMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Falloff"), TrimSettings.Falloff);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Roughness"), TrimSettings.Roughness);
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), TrimSettings.ToolOpacity);

	if (!CanEdit())
	{
		return;
	}

	const FVector Normal = UVoxelProjectionTools::GetHitsAverageNormal(Hits);
	
	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(Position);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;

	const FVoxelIntBox Bounds = UVoxelSphereTools::GetSphereBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		FVoxelMessages::Error("Invalid tool bounds!", &ToolManager);
		return;
	}
	const auto BoundsToCache = GetAndDebugBoundsToCache(World, Bounds, TickData);

	{
		auto& Data = World.GetData();

		FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);

		if (ToolManager.bCacheData) Data.CacheBounds<FVoxelValue>(BoundsToCache);

		UVoxelSphereTools::TrimSphereImpl(
			Data,
			VoxelPosition,
			Normal,
			VoxelRadius * (1 - TrimSettings.Falloff),
			VoxelRadius * TrimSettings.Falloff,
			!TickData.bAlternativeMode);
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}