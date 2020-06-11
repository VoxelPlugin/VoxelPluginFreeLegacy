// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Revert.h"
#include "VoxelTools/VoxelSphereTools.h"

#include "VoxelData/VoxelData.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings RevertToolSettings(const FVoxelToolManager_RevertSettings& RevertSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Revert Tool");

	Settings.Ptr_ToolMeshMaterial = &RevertSettings.ToolMaterial;

	Settings.bViewportSpaceMovement = true;
	Settings.Ptr_bShowPlanePreview = &RevertSettings.bShowPlanePreview;
	Settings.Ptr_Alignment = &RevertSettings.Alignment;
	Settings.Ptr_bAirMode = &RevertSettings.bAirMode;
	Settings.Ptr_DistanceToCamera = &RevertSettings.DistanceToCamera;
	
	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Revert::FVoxelToolManagerTool_Revert(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, RevertToolSettings(ToolManager.RevertSettings))
	, RevertSettings(ToolManager.RevertSettings)
{
}

void FVoxelToolManagerTool_Revert::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	const_cast<FVoxelToolManager_RevertSettings&>(RevertSettings).CurrentHistoryPosition = World.GetData().GetHistoryPosition();
	const_cast<FVoxelToolManager_RevertSettings&>(RevertSettings).HistoryPosition = FMath::Clamp(RevertSettings.HistoryPosition, 0, RevertSettings.CurrentHistoryPosition);
	
	if (!RevertSettings.SphereMesh)
	{
		FVoxelMessages::Error("VoxelToolManager: Revert Tool: Invalid SphereMesh!");
	}

	const float Scale = ToolManager.Radius / 50;
	const FTransform PreviewTransform(FQuat::Identity, GetToolPreviewPosition(), FVector(Scale));
	
	auto& MaterialInstance = GetToolMeshMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), RevertSettings.ToolOpacity);
	
	UpdateToolMesh(
		TickData.World,
		RevertSettings.SphereMesh,
		&MaterialInstance,
		PreviewTransform);

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
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

		if (TickData.bAlternativeMode)
		{
			UVoxelSphereTools::RevertSphereToGeneratorImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				RevertSettings.bRevertValues,
				RevertSettings.bRevertMaterials);
		}
		else
		{
			UVoxelSphereTools::RevertSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				RevertSettings.HistoryPosition,
				RevertSettings.bRevertValues,
				RevertSettings.bRevertMaterials);
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}