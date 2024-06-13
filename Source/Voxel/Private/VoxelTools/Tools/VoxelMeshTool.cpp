// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelTools/Tools/VoxelMeshTool.h"

#include "VoxelTools/VoxelDataTools.inl"
#include "VoxelTools/VoxelAssetTools.inl"
#include "VoxelData/VoxelData.h"
#include "VoxelUtilities/VoxelSDFUtilities.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelWorld.h"

#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

UVoxelMeshTool::UVoxelMeshTool()
{
	ToolName = TEXT("Mesh");
	bShowPaintMaterial = false;
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ToolMaterialFinder(TEXT("/Voxel/ToolMaterials/ToolMeshMaterial_Mesh"));
	ToolMaterial = ToolMaterialFinder.Object;
	
	ColorsMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_Color"));
	UVsMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_M_Chair_Emissive_UVs"));
	Mesh = FVoxelExampleUtilities::LoadExampleObject<UStaticMesh>(TEXT("/Voxel/Examples/Importers/Chair/VoxelExample_SM_Chair"));
}

void UVoxelMeshTool::GetToolConfig(FVoxelToolBaseConfig& OutConfig) const
{
	Super::GetToolConfig(OutConfig);

	OutConfig.MeshMaterial = ToolMaterial;
	OutConfig.Stride = Stride;
}

void UVoxelMeshTool::UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!MeshMaterialInstance)
	{
		return;
	}
	
	auto* MeshData = GetMeshData();
	if (!MeshData)
	{
		return;
	}

	FVector MeshScale;
	FTransform TransformNoTranslation;
	FTransform TransformWithTranslation;
	GetTransform(*MeshData, MeshScale, TransformNoTranslation, TransformWithTranslation);
	
	MeshMaterialInstance->SetScalarParameterValue(STATIC_FNAME("Opacity"), SharedConfig->ToolOpacity);
	
	UpdateToolMesh(
		Mesh,
		MeshMaterialInstance,
		TransformWithTranslation);
}

FVoxelIntBoxWithValidity UVoxelMeshTool::DoEdit()
{
	FVoxelMessages::Info("Using the mesh tool requires the Pro version of Voxel Plugin");
	return {};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const UVoxelMeshTool::FMeshData* UVoxelMeshTool::GetMeshData()
{
	return nullptr;
}

void UVoxelMeshTool::GetTransform(
	const FMeshData& MeshData, 
	FVector& OutMeshScale, 
	FTransform& OutTransformNoTranslation, 
	FTransform& OutTransformWithTranslation) const
{
	VOXEL_FUNCTION_COUNTER();
	
	OutMeshScale =
		bAbsoluteScale
		? Scale
		: SharedConfig->BrushSize * Scale / MeshData.Bounds.GetSize().GetMax();

	const auto GetRotationMatrix = [&]()
	{
		const FVector X = bAlignToMovement ? GetToolDirection() : FVector::ForwardVector;
		const FVector Z = bAlignToNormal ? GetToolNormal() : FVector::UpVector;
		const FVector Y = (Z ^ X).GetSafeNormal();
		return FMatrix(Y ^ Z, Y, Z, FVector(0));
	};

	// Matrix and Transform multiplications are left to right!

	const FMatrix ScaleMatrix = FScaleMatrix(OutMeshScale);
	const FVector ScaledPositionOffset = PositionOffset * ScaleMatrix.TransformVector(MeshData.Bounds.GetSize());

	OutTransformNoTranslation = FTransform(
		ScaleMatrix *
		FTranslationMatrix(ScaledPositionOffset) *
		FRotationMatrix(RotationOffset) *
		GetRotationMatrix());
	
	OutTransformWithTranslation = OutTransformNoTranslation * FTransform(GetToolPreviewPosition());
}