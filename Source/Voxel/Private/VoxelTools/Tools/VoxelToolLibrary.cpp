// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelToolLibary.h"
#include "VoxelTools/Tools/VoxelToolBase.h"

#include "Materials/MaterialInstanceDynamic.h"

void UVoxelToolLibrary::UpdateSphereOverlayMaterial(UVoxelToolBase* Tool, UMaterialInstanceDynamic* OverlayMaterialInstance, EVoxelFalloff FalloffType, float Falloff)
{
	VOXEL_FUNCTION_COUNTER();

	if (!Tool || !Tool->SharedConfig || !OverlayMaterialInstance)
	{
		return;
	}

	const FVector Position = Tool->GetToolPreviewPosition();

	const float Radius = Tool->SharedConfig->BrushSize / 2.f;
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Radius"), Radius);
	OverlayMaterialInstance->SetVectorParameterValue(STATIC_FNAME("Position"), Position);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), Tool->SharedConfig->ToolOpacity);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Falloff"), Falloff);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("EnableFalloff"), true);
	OverlayMaterialInstance->SetScalarParameterValue(STATIC_FNAME("FalloffType"), int32(FalloffType));

	Tool->SetToolOverlayBounds(FBox(Position - Radius, Position + Radius));
}