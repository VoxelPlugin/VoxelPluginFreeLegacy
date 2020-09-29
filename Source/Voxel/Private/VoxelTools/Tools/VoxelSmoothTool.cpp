// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelSmoothTool.h"
#include "VoxelTools/Tools/VoxelToolLibary.h"

#include "VoxelMessages.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"
#include "VoxelWorld.h"
#include "VoxelData/VoxelData.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UVoxelSmoothTool::UVoxelSmoothTool()
{
	ToolName = TEXT("Smooth");
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolRenderingMaterial_Smooth"));
	ToolMaterial = ToolMaterialFinder.Object;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSmoothTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	OutConfig.OverlayMaterial = ToolMaterial;
}

void UVoxelSmoothTool::Tick()
{
	Super::Tick();
	
	Falloff = GetValueAfterAxisInput(FVoxelToolAxes::Falloff, Falloff);
	Strength = GetValueAfterAxisInput(FVoxelToolAxes::Strength, Strength);
}

void UVoxelSmoothTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	UVoxelToolLibrary::UpdateSphereOverlayMaterial(this, OverlayMaterialInstance, FalloffType, Falloff);
}

FVoxelIntBoxWithValidity UVoxelSmoothTool::DoEdit()
{
	VOXEL_FUNCTION_COUNTER();

	auto& World = *GetVoxelWorld();

	const FVoxelVector VoxelPosition = World.GlobalToLocalFloat(GetToolPosition());
	const float VoxelRadius = SharedConfig->BrushSize / 2.f / World.VoxelSize;

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

	const int32 CurrentNumIterations = GetTickData().IsAlternativeMode() ? 1 : NumIterations;
	
	if (bSculpt)
	{
		FVoxelSphereToolsImpl::SmoothSphere(
			DataImpl,
			VoxelPosition,
			VoxelRadius,
			Strength,
			CurrentNumIterations,
			FalloffType,
			Falloff);
	}

	if (bPaint)
	{
		uint32 Mask = PaintMask;
		if (GetVoxelWorld()->MaterialConfig == EVoxelMaterialConfig::SingleIndex)
		{
			Mask &= ~EVoxelMaterialMask::SingleIndex;	
		}
		else if (GetVoxelWorld()->MaterialConfig == EVoxelMaterialConfig::MultiIndex)
		{
			FVoxelMessages::Warning("MultiIndex is not supported by smooth material", this);
			Mask = EVoxelMaterialMask::None;
		}
		
		FVoxelSphereToolsImpl::SmoothMaterialSphere(
			DataImpl,
			VoxelPosition,
			VoxelRadius,
			Strength,
			CurrentNumIterations,
			Mask,
			FalloffType,
			Falloff);
	}

	return Bounds;
}