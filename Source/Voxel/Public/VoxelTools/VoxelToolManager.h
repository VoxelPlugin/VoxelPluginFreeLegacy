// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGeneratorPicker.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelToolManager.generated.h"

class UVoxelWorldGenerator;
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
	Flatten,
	Trim,
	Level,
	Smooth,
	Sphere,
	Mesh,
	Revert,
	// Set the tool to this if you want to have no tool
	Custom UMETA(Hidden)
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

UENUM(BlueprintType)
enum class EVoxelToolManagerMaskType : uint8
{
	Texture,
	WorldGenerator
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
struct VOXEL_API FVoxelToolManager_SurfaceSettings
{
	GENERATED_BODY()

	FVoxelToolManager_SurfaceSettings();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	///////////////////////////////////////////////////////////////////////////

	// Will only affect the topmost voxels
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "2D Brush"))
	bool b2DBrush = false;
	
	// If true, sculpt/paint strength will be modulated by the distance the mouse travels
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bMovementAffectsStrength = false;
	
	// Relative to the radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin=0, UIMax=1))
	float Stride = 0.f;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bAlignToMovement = true;

	// If false, align the tool to the mouse movement
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bAlignToMovement"))
	FRotator FixedDirection = FRotator::ZeroRotator;

	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite)
	bool bEnableFalloff = true;
	
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff"))
	EVoxelToolManagerFalloff FalloffType = EVoxelToolManagerFalloff::Smooth;

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff", UIMin = "0", UIMax = "1"))
	float Falloff = 0.5;

	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite)
	bool bUseMask = false;
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask"))
	EVoxelToolManagerMaskType MaskType = EVoxelToolManagerMaskType::Texture;
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "Texture"))
	UTexture2D* MaskTexture = nullptr;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "Texture"))
	EVoxelRGBA MaskChannel = EVoxelRGBA::R;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "WorldGenerator"))
	FVoxelWorldGeneratorPicker MaskWorldGenerator;

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "WorldGenerator"))
	TArray<FName> SeedsToRandomize = { "Seed" };

	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "WorldGenerator"))
	bool bScaleWithRadius = true;
	
	UPROPERTY(Category = "Mask", VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Transient, meta = (EditCondition = "bUseMask", OnlyIfMaskType = "WorldGenerator"))
	UTexture2D* MaskWorldGeneratorDebugTexture = nullptr;
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask", UIMin = 0.01, UIMax = 10))
	float MaskScale = 1;

	// ScaleY/ScaleX. MaskScale = ScaleX
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (EditCondition = "bUseMask", UIMin = 0, UIMax = 10))
	float MaskRatio = 1;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;

	// Relative to brush size
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSculpt", UIMin = "0", UIMax = "1"))
	float SculptStrength = 0.1;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 0.1;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_FlattenSettings
{
	GENERATED_BODY()

	FVoxelToolManager_FlattenSettings();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	///////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Strength = 0.1;

	// If true, the plane used for flatten will be the same while clicking
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bFreezeOnClick = false;

	// Use Average Position & Normal
	// If true, use linetraces to find average position/normal under the cursor
	// If false, use a single linetrace from the cursor
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bUseAverage = true;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite)
	bool bEnableFalloff = true;
	
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff"))
	EVoxelToolManagerFalloff FalloffType = EVoxelToolManagerFalloff::Smooth;

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff", UIMin = "0", UIMax = "1"))
	float Falloff = 0.5;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_TrimSettings
{
	GENERATED_BODY()

	FVoxelToolManager_TrimSettings();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Falloff = 0.5;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Roughness = 0;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_LevelSettings
{
	GENERATED_BODY()

	FVoxelToolManager_LevelSettings();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UStaticMesh* CylinderMesh = nullptr;
	
	///////////////////////////////////////////////////////////////////////////
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Falloff = 0.1;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "10000"))
	float Height = 1000.f;

	// Offset, relative to the height
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Offset = 0.f;
	
	// Relative to the radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin=0, UIMax=1))
	float Stride = 0.f;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_SmoothSettings
{
	GENERATED_BODY()

	FVoxelToolManager_SmoothSettings();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0", UIMax = "1"))
	float Strength = 0.2;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_SphereSettingsBase
{
	GENERATED_BODY()

	FVoxelToolManager_SphereSettingsBase();
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UStaticMesh* SphereMesh = nullptr;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::View;

	// Position is based on the distance from the camera instead of the hit point
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	bool bAirMode = false;

	// Distance to the camera when no voxel world under the cursor, or Air Mode = true
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	float DistanceToCamera = 10000;

	UPROPERTY(Category = "Tool Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	bool bShowPlanePreview = true;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_SphereSettings : public FVoxelToolManager_SphereSettingsBase
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 1;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_MeshSettings
{
	GENERATED_BODY()

	FVoxelToolManager_MeshSettings();

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;
	
	///////////////////////////////////////////////////////////////////////////

	// Will ignore the radius, and only use the scale in the Transform category
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bUseMeshScale = false;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;

	// Relative to the radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin=0, UIMax=1))
	float Stride = 0.f;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	EVoxelToolManagerAlignment Alignment = EVoxelToolManagerAlignment::Surface;

	// Position is based on the distance from the camera instead of the hit point
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	bool bAirMode = false;

	// Distance to the camera when no voxel world under the cursor, or Air Mode = true
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	float DistanceToCamera = 10000;

	UPROPERTY(Category = "Tool Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (OnlyIfNotSurfaceAlignment))
	bool bShowPlanePreview = true;

	// Do a smooth import by converting the voxel densities & the mesh to true distance fields, and doing a smooth union/subtraction on these
	// Will disable painting
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bSmoothImport = false;

	// Relative to radius
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSmoothImport", UIMin = 0, UIMax = 1))
	float Smoothness = 0.5f;

	// Will slowly grow/shrink the surface towards the mesh
	// Will disabled SmoothImport
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bProgressiveStamp = false;

	// Speed of the progressive stamp
	// Make sure your mesh is intersecting the voxel world!
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bProgressiveStamp", UIMin = 0, UIMax = 1))
	float Speed = 0.1f;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;
	
	///////////////////////////////////////////////////////////////////////////

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = true;
	
	// Will sample ColorsMaterial at the mesh UVs to get the voxel colors
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint"))
	bool bPaintColors = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintColors"))
	UMaterialInterface* ColorsMaterial = nullptr;

	// Will sample UVChannelsMaterial at the mesh UVs to get the voxel UVs
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint"))
	bool bPaintUVs = true;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintUVs"))
	UMaterialInterface* UVsMaterial = nullptr;

	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", ShowForMaterialConfigs = "SingleIndex, DoubleIndex"))
	bool bPaintIndex = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint && bPaintIndex", ShowForMaterialConfigs = "SingleIndex, DoubleIndex"))
	uint8 Index = 0;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient)
	UTextureRenderTarget2D* UVsRenderTarget = nullptr;

	// For debug
	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, VisibleAnywhere, BlueprintReadOnly, Transient)
	UTextureRenderTarget2D* ColorsRenderTarget = nullptr;

	UPROPERTY(Category = "Paint Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint"))
	int32 RenderTargetSize = 4096;
	
	///////////////////////////////////////////////////////////////////////////

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

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelToolManager_RevertSettings : public FVoxelToolManager_SphereSettingsBase
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bRevertValues = true;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bRevertMaterials = false;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	int32 HistoryPosition = 0;

	UPROPERTY(Category = "Tool Settings", VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentHistoryPosition = 0;
};

UCLASS(BlueprintType, Blueprintable)
class VOXEL_API UVoxelToolManager : public UObject
{
	GENERATED_BODY()

public:
	UVoxelToolManager();
	
	void SaveConfig();
	void LoadConfig();
	
public:
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "All"))
	EVoxelToolManagerTool Tool;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "All", ClampMin = "0", UIMin = "0", UIMax = "10000"))
	float Radius = 500;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	float RadiusChangeSpeed = 100;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "Surface, Mesh", UIMin = 0, UIMax = 1))
	float AlignToMovementSmoothness = 0.75;

	// If empty, allow editing all worlds
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Transient, meta = (ShowForTools = "All"))
	TArray<AVoxelWorld*> WorldsToEdit;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bCacheData = true;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bRegenerateSpawners = true;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bCheckForSingleValues = true;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bWaitForUpdates = true;
	
	UPROPERTY(Category = "Voxel Editor", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (ShowForTools = "All"))
	bool bDebug = false;

public:
	// Used by viewport alignment
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UStaticMesh* PlaneMesh = nullptr;

	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* PlaneMaterial = nullptr;
	
public:
	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Surface", ShowOnlyInnerProperties))
	FVoxelToolManager_SurfaceSettings SurfaceSettings;
	
	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Flatten", ShowOnlyInnerProperties))
	FVoxelToolManager_FlattenSettings FlattenSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Trim", ShowOnlyInnerProperties))
	FVoxelToolManager_TrimSettings TrimSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Level", ShowOnlyInnerProperties))
	FVoxelToolManager_LevelSettings LevelSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Smooth", ShowOnlyInnerProperties))
	FVoxelToolManager_SmoothSettings SmoothSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Sphere", ShowOnlyInnerProperties))
	FVoxelToolManager_SphereSettings SphereSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Mesh", ShowOnlyInnerProperties))
	FVoxelToolManager_MeshSettings MeshSettings;

	UPROPERTY(Category = "Voxel", EditAnywhere, BlueprintReadWrite, meta = (ShowForTools = "Revert", ShowOnlyInnerProperties))
	FVoxelToolManager_RevertSettings RevertSettings;
	
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
	void ClearToolInstance();

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

	// Needed to still tick the tools so that the preview is updated when changing settings
	UPROPERTY(Transient)
	FVector2D SimpleTick_LastKnownMousePosition = FVector2D::ZeroVector;
};