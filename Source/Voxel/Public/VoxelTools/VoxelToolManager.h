// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelToolManager.generated.h"

struct FHitResult;
class FVoxelToolManagerTool;
class UMaterialInterface;
class UTexture2D;
class UTextureRenderTarget2D;
class UStaticMesh;
class AVoxelWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoxelToolManager_OnBoundsUpdated, AVoxelWorld*, World, FIntBox, Bounds);

UENUM(BlueprintType)
enum class EVoxelToolManagerTool : uint8
{
	Surface,
	Trim,
	Smooth,
	Sphere,
	Mesh
};

UENUM(BlueprintType)
enum class EVoxelToolManagerFalloff : uint8
{
	Linear,
	Smooth,
	Spherical,
	Tip
};

UENUM(BlueprintType)
enum class EVoxelToolManagerAlignment : uint8
{
	// The tool follow the surface. The surface is frozen until the next click.
	Surface,
	// Align with the camera view plane
	View,
	// Align with XY plane
	Ground,
	// Align with the camera view plane, with the Z component zeroed out
	Up
};

USTRUCT(BlueprintType)
struct FVoxelToolManagerTickData
{
	GENERATED_BODY()

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	UWorld* World = nullptr;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	float DeltaTime = 0;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	FVector2D MousePosition = FVector2D::ZeroVector;
	
	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	FVector CameraViewDirection = FVector::ForwardVector;
	
	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	FVector RayOrigin = FVector::ZeroVector;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	FVector RayDirection = FVector::ForwardVector;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	bool bClick = false;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	bool bAlternativeMode = false;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite)
	float MouseWheelDelta = 0;
};


USTRUCT(BlueprintType)
struct FVoxelToolManager_SurfaceSettings
{
	GENERATED_BODY()

	FVoxelToolManager_SurfaceSettings();

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
public:
	// If true, sculpt/paint strength will be modulated by the distance the mouse travels
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bMovementAffectsStrength = false;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bEnableStride = false;

	// In seconds
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableStride", UIMin=0.01, UIMax=1))
	float Stride = 0.1;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bAlignToMovement = true;

	// If false, align the tool to the mouse movement
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bAlignToMovement"))
	FRotator FixedDirection = FRotator::ZeroRotator;

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite)
	EVoxelToolManagerFalloff FalloffType = EVoxelToolManagerFalloff::Smooth;

	// Relative to the radius
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "10"))
	float Falloff = 0.5;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite)
	bool bUseMask = false;
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask"))
	UTexture2D* Mask = nullptr;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask"))
	EVoxelRGBA MaskChannel = EVoxelRGBA::R;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", UIMin = 0.01, UIMax = 10))
	float MaskScale = 1;

	// ScaleY/ScaleX. MaskScale = ScaleX
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (EditCondition = "bUseMask", UIMin = 0, UIMax = 10))
	float MaskRatio = 1;
	
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;
	
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSculpt", UIMin = "0", UIMax = "1"))
	float SculptStrength = 0.1;

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 0.1;
};

USTRUCT(BlueprintType)
struct FVoxelToolManager_TrimSettings
{
	GENERATED_BODY()

	FVoxelToolManager_TrimSettings();

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	// Relative to the radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "10"))
	float Falloff = 0.5;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Roughness = 0;
};

USTRUCT(BlueprintType)
struct FVoxelToolManager_SmoothSettings
{
	GENERATED_BODY()

	FVoxelToolManager_SmoothSettings();
	
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Strength = 0.2;
};

USTRUCT(BlueprintType)
struct FVoxelToolManager_SphereSettings
{
	GENERATED_BODY()

	FVoxelToolManager_SphereSettings();

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UStaticMesh* SphereMesh = nullptr;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::View;

	// Position is based on the distance from the camera instead of the hit point
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment = "Sphere"))
	bool bAirMode = false;

	// Distance to the camera when no voxel world under the cursor, or Air Mode = true
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment = "Sphere"))
	float DistanceToCamera = 10000;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 1;
};

USTRUCT(BlueprintType)
struct FVoxelToolManager_MeshSettings
{
	GENERATED_BODY()

	FVoxelToolManager_MeshSettings();

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

	// Will ignore the radius, and only use the scale in the Transform category
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bUseMeshScale = false;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bEnableStride = false;

	// In seconds
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableStride", UIMin=0.01, UIMax=1))
	float Stride = 0.1;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::Surface;

	// Position is based on the distance from the camera instead of the hit point
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment = "Mesh"))
	bool bAirMode = false;

	// Distance to the camera when no voxel world under the cursor, or Air Mode = true
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment = "Mesh"))
	float DistanceToCamera = 10000;

public:
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;

public:
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = true;
	
	// Will sample ColorsMaterial at the mesh UVs to get the voxel colors
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaintColors = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaintColors"))
	UMaterialInterface* ColorsMaterial = nullptr;

	// Will sample UVChannelsMaterial at the mesh UVs to get the voxel UVs
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaintUVs = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaintUVs"))
	UMaterialInterface* UVsMaterial = nullptr;

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bSetIndex = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (ShowForMaterialConfigs = "SingleIndex, DoubleIndex", EditCondition = bSetIndex))
	uint8 PaintIndex = 0;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient)
	UTextureRenderTarget2D* UVsRenderTarget = nullptr;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient)
	UTextureRenderTarget2D* ColorsRenderTarget = nullptr;

	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite)
	int32 RenderTargetSize = 4096;
	
public:
	// Relative to the size of the mesh
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite, meta = (UIMin = -1, UIMax = 1))
	FVector PositionOffset = FVector::ZeroVector;
	
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	bool bAlignToNormal = true;

	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	bool bAlignToMovement = true;

	// Applied after position and scale offset
	UPROPERTY(Category = "Transform", EditAnywhere, BlueprintReadWrite)
	FRotator RotationOffset = FRotator::ZeroRotator;
};

UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelToolManager : public UObject
{
	GENERATED_BODY()

public:
	void SaveConfig();
	void LoadConfig();
	
public:
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "All"))
	EVoxelToolManagerTool Tool;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "All", ClampMin = "0", UIMin = "0", UIMax = "10000"))
	float Radius = 500;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bDebug = false;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	float RadiusChangeSpeed = 100;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "Surface, Mesh", UIMin = 0, UIMax = 1))
	float AlignToMovementSmoothness = 0.75;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Surface", ShowOnlyInnerProperties))
	FVoxelToolManager_SurfaceSettings SurfaceSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Trim", ShowOnlyInnerProperties))
	FVoxelToolManager_TrimSettings TrimSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Smooth", ShowOnlyInnerProperties))
	FVoxelToolManager_SmoothSettings SmoothSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Sphere", ShowOnlyInnerProperties))
	FVoxelToolManager_SphereSettings SphereSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Mesh", ShowOnlyInnerProperties))
	FVoxelToolManager_MeshSettings MeshSettings;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Surface, Sphere", ShowOnlyInnerProperties))
	FVoxelPaintMaterial PaintMaterial;

public:
	UPROPERTY(BlueprintAssignable, Category = "Voxel")
	FVoxelToolManager_OnBoundsUpdated OnBoundsUpdated;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void Tick(FVoxelToolManagerTickData TickData);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SimpleTick(APlayerController* PlayerController, bool bClick, bool bAlternativeMode, float MouseWheelDelta);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void Destroy();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void RecreateToolInstance();

public:
#if WITH_EDITOR
	bool IsPropertyVisible(const UProperty& Property, const UProperty* ParentProperty) const;

	FSimpleMulticastDelegate RefreshDetails;
#endif
	
protected:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif
	//~ End UObject Interface

public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FRegisterTransactionDelegate, FName, AVoxelWorld*);
	FRegisterTransactionDelegate RegisterTransaction;
	
	void SaveFrame(AVoxelWorld& World, const FIntBox& Bounds, FName Name) const;
	
private:
	EVoxelToolManagerTool ToolInstanceType = EVoxelToolManagerTool::Surface;
	TVoxelSharedPtr<FVoxelToolManagerTool> ToolInstance;

	UPROPERTY(Transient)
	AVoxelWorld* VoxelWorld;
};