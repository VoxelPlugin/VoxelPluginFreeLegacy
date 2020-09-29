// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelSphereTool.h"
#include "VoxelTools/Tools/VoxelToolLibary.h"

#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"
#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.inl"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelData/VoxelData.h"
#include "VoxelWorld.h"

#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelSphereTool::UVoxelSphereTool()
{
	ToolName = TEXT("Sphere");
	bShowPaintMaterial = true;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OverlayMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Sphere"));
	OverlayMaterial = OverlayMaterialFinder.Object;
}

void UVoxelSphereTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	Super::GetToolConfig(OutConfig);

	if (bPaint && !bSculpt)
	{
		OutConfig.OverlayMaterial = OverlayMaterial;
	}
}

void UVoxelSphereTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	Super::UpdateRender(OverlayMaterialInstance, MeshMaterialInstance);
	
	UVoxelToolLibrary::UpdateSphereOverlayMaterial(this, OverlayMaterialInstance, FalloffType, Falloff);
}

FVoxelIntBoxWithValidity UVoxelSphereTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	AVoxelWorld& World = *GetVoxelWorld();
	
	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = SharedConfig->BrushSize / 2.f / World.VoxelSize;

	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(VoxelPosition, VoxelRadius);
	if (!Bounds.IsValid())
	{
		return {};
	}
	const auto BoundsToCache = GetBoundsToCache(Bounds);

	FVoxelData& Data = World.GetData();
	auto DataImpl = GetDataImpl(Data);

	FVoxelWriteScopeLock Lock(Data, BoundsToCache, FUNCTION_FNAME);
	CacheData<FVoxelValue>(Data, BoundsToCache);

	if (bSculpt)
	{
		if (GetTickData().IsAlternativeMode())
		{
			FVoxelSphereToolsImpl::RemoveSphere(
				DataImpl,
				VoxelPosition,
				VoxelRadius);
		}
		else
		{
			if (bPaint)
			{
				auto RecordingDataImpl = GetDataImpl<FModifiedVoxelValue>(Data);
				FVoxelSphereToolsImpl::AddSphere(
					RecordingDataImpl,
					VoxelPosition,
					VoxelRadius);

				TArray<FVoxelSurfaceEditsVoxel> Materials;
				Materials.Reserve(RecordingDataImpl.ModifiedValues.Num());
				for (auto& Voxel : RecordingDataImpl.ModifiedValues)
				{
					if (Voxel.OldValue > 0 && Voxel.NewValue <= 0)
					{
						FVoxelSurfaceEditsVoxel NewVoxel;
						NewVoxel.Position = Voxel.Position;
						NewVoxel.Strength = PaintStrength;
						Materials.Emplace(NewVoxel);
					}
				}
				FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(DataImpl, Bounds, SharedConfig->PaintMaterial, Materials);
			}
			else
			{
				FVoxelSphereToolsImpl::AddSphere(
					DataImpl,
					VoxelPosition,
					VoxelRadius);
			}
		}
	}
	else if (bPaint)
	{
		const float Strength = GetTickData().IsAlternativeMode() ? -PaintStrength : PaintStrength;

		FVoxelSphereToolsImpl::SetMaterialSphere(
			DataImpl,
			VoxelPosition,
			VoxelRadius,
			SharedConfig->PaintMaterial,
			Strength,
			FalloffType,
			Falloff);
	}

	return Bounds;
}