// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Level.h"

#include "VoxelData/VoxelData.h"
#include "VoxelTools/VoxelDataTools.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings LevelToolSettings(const FVoxelToolManager_LevelSettings& LevelSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Level Tool");

	Settings.Ptr_ToolMeshMaterial = &LevelSettings.ToolMaterial;
	
	Settings.Ptr_Stride = &LevelSettings.Stride;

	Settings.bViewportSpaceMovement = true;

	static EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::Ground;
	static bool bAirMode = false;
	static float DistanceToCamera = 1e5;

	Settings.Ptr_Alignment = &Alignment;
	Settings.Ptr_bAirMode = &bAirMode;
	Settings.Ptr_DistanceToCamera = &DistanceToCamera;

	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Level::FVoxelToolManagerTool_Level(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, LevelToolSettings(ToolManager.LevelSettings))
	, LevelSettings(ToolManager.LevelSettings)
{
}

void FVoxelToolManagerTool_Level::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	const FVector ToolOffset = FVector(0.f, 0.f,  LevelSettings.Offset * LevelSettings.Height * (TickData.bAlternativeMode ? -1 : 1));
	
	const float ScaleXY = ToolManager.Radius / 50.f;
	const float ScaleZ = LevelSettings.Height / 100.f + 0.001f;
	const FTransform PreviewTransform(
		FQuat::Identity,
		GetToolPreviewPosition() + ToolOffset + FVector(0, 0, (TickData.bAlternativeMode ? LevelSettings.Height : -LevelSettings.Height) / 2),
		FVector(ScaleXY, ScaleXY, ScaleZ));
	
	auto& MaterialInstance = GetToolMeshMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), LevelSettings.ToolOpacity);
	
	UpdateToolMesh(
		TickData.World,
		LevelSettings.CylinderMesh,
		&MaterialInstance,
		PreviewTransform);

	if (!CanEdit())
	{
		return;
	}

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition() + ToolOffset);
	const float VoxelRadius = ToolManager.Radius / World.VoxelSize;
	const float VoxelHeight = LevelSettings.Height / World.VoxelSize;

	const FVoxelIntBox Bounds = UVoxelDataTools::GetLevelToolBounds(VoxelPosition, VoxelRadius, VoxelHeight, !TickData.bAlternativeMode);
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

		UVoxelDataTools::LevelImpl(
			Data,
			VoxelPosition,
			VoxelRadius,
			LevelSettings.Falloff,
			VoxelHeight,
			!TickData.bAlternativeMode);
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}