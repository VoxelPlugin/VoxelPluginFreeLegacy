// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelTexture.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelTools/Tools/VoxelToolBase.h"
#include "VoxelSurfaceTool.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EVoxelSurfaceToolMaskType : uint8
{
	Texture,
	Generator
};

USTRUCT(BlueprintType)
struct FVoxelSurfaceToolMask
{
	GENERATED_BODY()
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite)
	EVoxelSurfaceToolMaskType Type = EVoxelSurfaceToolMaskType::Texture;

public:
	UPROPERTY(Category = "Mask|Texture", EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Texture = nullptr;

	UPROPERTY(Category = "Mask|Texture", EditAnywhere, BlueprintReadWrite)
	EVoxelRGBA Channel = EVoxelRGBA::R;

public:
	UPROPERTY(Category = "Mask|Generator", EditAnywhere, BlueprintReadWrite)
	FVoxelGeneratorPicker Generator;

	UPROPERTY(Category = "Mask|Generator", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	TArray<FName> SeedsToRandomize = { "Seed" };

	UPROPERTY(Category = "Mask|Generator", EditAnywhere, BlueprintReadWrite)
	bool bScaleWithBrushSize = true;
	
	UPROPERTY(Category = "Mask|Generator", VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, Transient)
	TObjectPtr<UTexture2D> GeneratorDebugTexture = nullptr;
	
public:
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0.01, UIMax = 10))
	float Scale = 1;

	// ScaleY/ScaleX. MaskScale = ScaleX
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, AdvancedDisplay, meta = (UIMin = 0, UIMax = 10))
	float Ratio = 1;

public:
	bool HasSameGeneratorSettings(const FVoxelSurfaceToolMask& Other) const
	{
		return
			Generator.GetObject() == Other.Generator.GetObject() &&
			SeedsToRandomize == Other.SeedsToRandomize &&
			bScaleWithBrushSize == Other.bScaleWithBrushSize &&
			Scale == Other.Scale &&
			Ratio == Other.Ratio;
	}
};

UCLASS()
class VOXEL_API UVoxelSurfaceTool : public UVoxelToolBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	TObjectPtr<UMaterialInterface> ToolMaterial = nullptr;
	
public:
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite)
	bool bSculpt = true;

	// Relative to brush size
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSculpt", UIMin = "0", UIMax = "1"))
	float SculptStrength = 0.5f;

	// If true, will propagate materials so that the surface stays correctly painted.
	// Disabled in 2D mode or if Paint is enabled
	UPROPERTY(Category = "Sculpt Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "!bPaint && !b2DBrush"))
	bool bPropagateMaterials = true;
	
public:
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 0.5f;
	
public:
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

	// If true, strength will be modulated by the time since the last edit so that the results don't depend on the framerate
	// Automatically turned off if Stride > 0, or if paint strength = 1
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool bModulateStrengthByDeltaTime = true;
	
public:
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite)
	bool bEnableFalloff = true;
	
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff"))
	EVoxelFalloff FalloffType = EVoxelFalloff::Smooth;

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff", UIMin = "0", UIMax = "1"))
	float Falloff = 0.5;
	
public:
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite)
	bool bUseMask = false;
	
	UPROPERTY(Category = "Mask", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseMask"))
	FVoxelSurfaceToolMask Mask;

public:
	UVoxelSurfaceTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const override;
	virtual void Tick() override;
	virtual void UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance) override;
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface

private:
	struct FMaskGeneratorCache
	{
		FVoxelSurfaceToolMask CachedConfig;
		float BrushSize = 0.f; // Needed for bScaleWithBrushSize
		
		TMap<FName, int32> Seeds;

		TVoxelTexture<float> VoxelTexture;
	};
	FMaskGeneratorCache MaskGeneratorCache;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> MaskGeneratorCache_RenderTexture = nullptr;

	bool ShouldUseMask() const;

	void GetStrengths(float& OutSignedSculptStrength, float& OutSignedPaintStrength) const;
};