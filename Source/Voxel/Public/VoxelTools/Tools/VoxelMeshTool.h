// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelDataAssetData.h"
#include "VoxelImporters/VoxelMeshImporter.h"
#include "VoxelTools/Tools/VoxelToolWithAlignment.h"
#include "VoxelMeshTool.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class VOXEL_API UVoxelMeshTool : public UVoxelToolWithAlignment
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
public:
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	// Relative to the radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin=0, UIMax=1))
	float Stride = 0.f;

	// Do a smooth import by converting the voxel densities & the mesh to true distance fields, and doing a smooth union/subtraction on these
	// NOTE: Disabled if bProgressiveStamp = true
	// NOTE: Will disable bImportColorsFromMesh/bImportUVsFromMesh
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bProgressiveStamp"))
	bool bSmoothImport = false;

	// Relative to radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSmoothImport && !bProgressiveStamp", UIMin = 0, UIMax = 1))
	float Smoothness = 0.5f;

	// Will slowly grow/shrink the surface towards the mesh
	// NOTE: Will disable SmoothImport
	// NOTE: Will disable bImportColorsFromMesh/bImportUVsFromMesh
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bProgressiveStamp = false;

	// Speed of the progressive stamp
	// Make sure your mesh is intersecting the voxel world!
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bProgressiveStamp", UIMin = 0, UIMax = 1))
	float Speed = 0.1f;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	FVoxelMeshImporterSettingsBase MeshImporterSettings;
	
public:
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;
	
public:
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = true;

	// Use to restrict editing on some channels
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", Bitmask, BitmaskEnum = EVoxelMaterialMask_BP))
	int32 PaintMask = EVoxelMaterialMask::All;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", ShowForMaterialConfigs = "RGB, SingleIndex"))
	bool bPaintColors = true;

	// Import the colors directly from the mesh by sampling ColorsMaterial at the mesh UVs
	// NOTE: Will be disabled if bSmoothImport = true or bProgressiveStamp = true
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && !bSmoothImport && !bProgressiveStamp && bPaintColors", ShowForMaterialConfigs = "RGB, SingleIndex"))
	bool bImportColorsFromMesh = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && !bSmoothImport && !bProgressiveStamp && bPaintColors && bImportColorsFromMesh", ShowForMaterialConfigs = "RGB, SingleIndex"))
	UMaterialInterface* ColorsMaterial = nullptr;

	// Used if bImportColorsFromMesh = false
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintColors", ShowForMaterialConfigs = "RGB, SingleIndex"))
	FColor ColorToPaint = FColor::White;

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", ShowForMaterialConfigs = "RGB, SingleIndex"))
	bool bPaintUVs = true;

	// Import the uvs directly from the mesh by sampling UVsMaterial at the mesh UVs
	// NOTE: Will be disabled if bSmoothImport = true or bProgressiveStamp = true
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && !bSmoothImport && !bProgressiveStamp && bPaintUVs", ShowForMaterialConfigs = "RGB, SingleIndex"))
	bool bImportUVsFromMesh = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && !bSmoothImport && !bProgressiveStamp && bPaintUVs", ShowForMaterialConfigs = "RGB, SingleIndex"))
	UMaterialInterface* UVsMaterial = nullptr;

	// Used if bImportUVsFromMesh = false
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintUVs", ShowForMaterialConfigs = "RGB, SingleIndex"))
	FVector2D UV0ToPaint = FVector2D::ZeroVector;

	// Used if bImportUVsFromMesh = false
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintUVs", ShowForMaterialConfigs = "RGB, SingleIndex"))
	FVector2D UV1ToPaint = FVector2D::ZeroVector;

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", ShowForMaterialConfigs = "SingleIndex, MultiIndex"))
	bool bPaintIndex = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintIndex", ShowForMaterialConfigs = "SingleIndex, MultiIndex"))
	uint8 IndexToPaint = 0;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient, meta = (ShowForMaterialConfigs = "RGB, SingleIndex"))
	UTextureRenderTarget2D* UVsRenderTarget = nullptr;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient, meta = (ShowForMaterialConfigs = "RGB, SingleIndex"))
	UTextureRenderTarget2D* ColorsRenderTarget = nullptr;

	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && !bSmoothImport && !bProgressiveStamp"), meta = (ShowForMaterialConfigs = "RGB, SingleIndex"))
	int32 RenderTargetSize = 4096;
	
public:
	// Relative to the size of the mesh
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite, meta = (UIMin = -1, UIMax = 1))
	FVector PositionOffset = FVector::ZeroVector;
	
	// If false the mesh scale will be set to match the radius
	// If true the mesh scale will ignore the radius, and only use the scale below
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	bool bAbsoluteScale = false;
	
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	bool bAlignToNormal = true;

	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	bool bAlignToMovement = true;

	// Applied after position and scale offset
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;

public:
	UVoxelMeshTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const override;
	virtual void UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance) override;
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface

private:
	struct FMeshData
	{
		TWeakObjectPtr<UStaticMesh> StaticMesh;
		FVoxelMeshImporterInputData Data;
		FBox Bounds;
	};
	TUniquePtr<const FMeshData> CachedMeshData;

	const FMeshData* GetMeshData();
	void GetTransform(
		const FMeshData& MeshData,
		FVector& OutMeshScale,
		FTransform& OutTransformNoTranslation,
		FTransform& OutTransformWithTranslation) const;
	
private:
	struct FAssetData
	{
		FTransform Transform;
		FVoxelDataAssetData Data;
		FIntVector PositionOffset = FIntVector::ZeroValue;
	};
	struct FDistanceFieldData
	{
		FTransform Transform;
		float BoxExtension = 0;
		FVoxelMeshImporterSettingsBase ImporterSettings;
		
		TArray<float> Data;
		TArray<FVector> SurfacePositions;
		FIntVector Size;
		FIntVector PositionOffset = FIntVector::ZeroValue;
	};
	TUniquePtr<const FAssetData> AssetData;
	TUniquePtr<const FDistanceFieldData> DistanceFieldData;

	UPROPERTY(Transient)
	FVoxelMeshImporterRenderTargetCache RenderTargetCache;
	
	UPROPERTY(Transient)
	FVoxelMeshImporterSettings AssetData_ImporterSettings;
};