// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Tools/VoxelToolBase.h"
#include "VoxelSmoothTool.generated.h"

UCLASS()
class VOXEL_API UVoxelSmoothTool : public UVoxelToolBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Tool Preview Settings", EditAnywhere, BlueprintReadWrite, meta = (HideInPanel))
	UMaterialInterface* ToolMaterial = nullptr;

public:
	// NumIterations also affects strength
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 1))
	float Strength = 1.f;

	// Number of times to apply the smooth in a single frame
	// Will be ignored if Shift is pressed
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 1))
	int32 NumIterations = 10;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	EVoxelFalloff FalloffType = EVoxelFalloff::Smooth;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (UIMin = 0, UIMax = 1))
	float Falloff = 0.5f;
	
public:
	UVoxelSmoothTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const override;
	virtual void Tick() override;
	virtual void UpdateRender(UMaterialInstanceDynamic* OverlayMaterialInstance, UMaterialInstanceDynamic* MeshMaterialInstance) override;
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface
};