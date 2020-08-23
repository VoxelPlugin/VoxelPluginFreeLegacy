// Copyright 2020 Phyronnaz

#include "VoxelTools/Tools/VoxelSphereToolBase.h"

#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelSphereToolBase::UVoxelSphereToolBase()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	ToolMaterial = ToolMaterialFinder.Object;
	SphereMesh = SphereMeshFinder.Object;
}

void UVoxelSphereToolBase::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	Super::GetToolConfig(OutConfig);
	
	OutConfig.MeshMaterial = ToolMaterial;
}

void UVoxelSphereToolBase::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();

	if (!MeshMaterialInstance)
	{
		return;
	}

	const float Scale = SharedConfig->BrushSize / 100.f;
	const FTransform PreviewTransform(FQuat::Identity, GetToolPreviewPosition(), FVector(Scale));

	MeshMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);

	UpdateToolMesh(SphereMesh, MeshMaterialInstance, PreviewTransform);
}