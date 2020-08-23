// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Tools/VoxelSphereToolBase.h"
#include "VoxelSphereTool.generated.h"

UCLASS()
class VOXEL_API UVoxelSphereTool : public UVoxelSphereToolBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite)
	bool bPaint = false;
	
	UPROPERTY(Category = "Paint Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bPaint", UIMin = "0", UIMax = "1"))
	float PaintStrength = 1;

public:
	UVoxelSphereTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface
};