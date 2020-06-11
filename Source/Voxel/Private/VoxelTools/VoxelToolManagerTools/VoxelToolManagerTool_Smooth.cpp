// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Smooth.h"
#include "VoxelTools/VoxelSphereTools.h"

#include "VoxelData/VoxelData.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"

#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings SmoothToolSettings(const FVoxelToolManager_SmoothSettings& SmoothSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Smooth Tool");

	Settings.Ptr_ToolOverlayMaterial = &SmoothSettings.ToolMaterial;

	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Smooth::FVoxelToolManagerTool_Smooth(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, SmoothToolSettings(ToolManager.SmoothSettings))
	, SmoothSettings(ToolManager.SmoothSettings)
{
}

void FVoxelToolManagerTool_Smooth::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();
	
	SetToolRenderingBounds(World, FBox(GetToolPreviewPosition() - ToolManager.Radius, GetToolPreviewPosition() + ToolManager.Radius));

	auto& MaterialInstance = GetToolOverlayMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Radius"), ToolManager.Radius);
	MaterialInstance.SetVectorParameterValue(STATIC_FNAME("Position"), GetToolPreviewPosition());
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), SmoothSettings.ToolOpacity);

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
			UVoxelSphereTools::SharpenSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				SmoothSettings.Strength);
		}
		else
		{
			UVoxelSphereTools::SmoothSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius,
				SmoothSettings.Strength);
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}