// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelToolManagerTools/VoxelToolManagerTool_Sphere.h"
#include "VoxelTools/VoxelSphereTools.h"
#include "VoxelTools/VoxelSurfaceTools.h"

#include "VoxelData/VoxelData.h"

#include "VoxelWorld.h"
#include "VoxelMessages.h"
#include "Materials/MaterialInstanceDynamic.h"

inline FVoxelToolManagerTool::FToolSettings SphereToolSettings(const FVoxelToolManager_SphereSettings& SphereSettings)
{
	FVoxelToolManagerTool::FToolSettings Settings;

	Settings.ToolName = STATIC_FNAME("Sphere Tool");

	Settings.Ptr_ToolMeshMaterial = &SphereSettings.ToolMaterial;
	
	Settings.bViewportSpaceMovement = true;
	Settings.Ptr_bShowPlanePreview = &SphereSettings.bShowPlanePreview;
	Settings.Ptr_Alignment = &SphereSettings.Alignment;
	Settings.Ptr_bAirMode = &SphereSettings.bAirMode;
	Settings.Ptr_DistanceToCamera = &SphereSettings.DistanceToCamera;
	
	return Settings;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelToolManagerTool_Sphere::FVoxelToolManagerTool_Sphere(const UVoxelToolManager& ToolManager)
	: FVoxelToolManagerTool(ToolManager, SphereToolSettings(ToolManager.SphereSettings))
	, SphereSettings(ToolManager.SphereSettings)
{
}

void FVoxelToolManagerTool_Sphere::Tick(AVoxelWorld& World, const FVoxelToolManagerTickData& TickData)
{
	VOXEL_FUNCTION_COUNTER();

	if (!SphereSettings.SphereMesh)
	{
		FVoxelMessages::Error("VoxelToolManager: Sphere Tool: Invalid SphereMesh!");
	}

	const float Scale = ToolManager.Radius / 50;
	const FTransform PreviewTransform(FQuat::Identity, GetToolPreviewPosition(), FVector(Scale));
	
	auto& MaterialInstance = GetToolMeshMaterialInstance();
	MaterialInstance.SetScalarParameterValue(STATIC_FNAME("Opacity"), SphereSettings.ToolOpacity);
	
	UpdateToolMesh(
		TickData.World,
		SphereSettings.SphereMesh,
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
			UVoxelSphereTools::RemoveSphereImpl(
				Data,
				VoxelPosition,
				VoxelRadius);
		}
		else
		{
			if (SphereSettings.bPaint)
			{
				TArray<FModifiedVoxelValue> ModifiedVoxels;
				UVoxelSphereTools::AddSphereImpl(
					Data,
					VoxelPosition,
					VoxelRadius,
					ModifiedVoxels);

				TArray<FVoxelSurfaceEditsVoxel> Materials;
				Materials.Reserve(ModifiedVoxels.Num());
				for (auto& Voxel : ModifiedVoxels)
				{
					if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
					{
						FVoxelSurfaceEditsVoxel NewVoxel;
						NewVoxel.Position = Voxel.Position;
						NewVoxel.Strength = SphereSettings.PaintStrength;
						Materials.Emplace(NewVoxel);
					}
				}
				UVoxelSurfaceTools::EditVoxelMaterialsImpl(Data, Bounds, ToolManager.PaintMaterial, Materials);
			}
			else
			{
				UVoxelSphereTools::AddSphereImpl(
					Data,
					VoxelPosition,
					VoxelRadius);
			}
		}
	}

	SaveFrameOnEndClick(Bounds);
	UpdateWorld(World, Bounds);
}