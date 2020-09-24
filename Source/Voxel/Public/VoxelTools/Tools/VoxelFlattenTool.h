// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Tools/VoxelToolBase.h"
#include "VoxelFlattenTool.generated.h"

UCLASS()
class VOXEL_API UVoxelFlattenTool : public UVoxelToolBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

public:
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

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bUseFixedRotation = false;
	
	// Override the normal by a fixed rotation
	// The rotation is apply to Up Vector to find the plane normal
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseFixedRotation"))
	FRotator FixedRotation { ForceInit };
	
	// If true, will propagate materials so that the surface stays correctly painted. Expensive.
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bPropagateMaterials = true;
	
public:
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite)
	bool bEnableFalloff = true;
	
	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff"))
	EVoxelFalloff FalloffType = EVoxelFalloff::Smooth;

	UPROPERTY(Category = "Falloff", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableFalloff", UIMin = "0", UIMax = "1"))
	float Falloff = 0.5;

public:
	UVoxelFlattenTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const override;
	virtual void Tick() override;
	virtual void UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance) override;
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface

private:
	FVector LastClickFlattenPosition = FVector::ZeroVector;
	FVector LastClickFlattenNormal = FVector::UpVector;
};