// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Tools/VoxelToolBase.h"
#include "VoxelToolWithAlignment.generated.h"

// Voxel tool with basic alignment settings
UCLASS(Abstract)
class VOXEL_API UVoxelToolWithAlignment : public UVoxelToolBase
{
	GENERATED_BODY()

public:
	// The plane your sculpting is restricted to when holding mouse button down
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	EVoxelToolAlignment Alignment = EVoxelToolAlignment::View;
	
	// Position is based on the distance from the camera instead of the hit point
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Alignment != EVoxelToolAlignment::Surface"))
	bool bAirMode = false;

	// Distance to the camera when no voxel world under the cursor, or Air Mode = true
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Alignment != EVoxelToolAlignment::Surface"))
	float DistanceToCamera = 10000;

	UPROPERTY(Category = "Tool Settings", AdvancedDisplay, EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Alignment != EVoxelToolAlignment::Surface"))
	bool bShowPlanePreview = true;

public:
	//~ Begin UVoxelToolBase Interface
	virtual void GetToolConfig(FVoxelToolBaseConfig& OutConfig) const override
	{
		OutConfig.bHasAlignment = true;
		OutConfig.Alignment = Alignment;
		OutConfig.bAirMode = bAirMode;
		OutConfig.DistanceToCamera = DistanceToCamera;
		OutConfig.bShowPlanePreview = bShowPlanePreview;
	}
	//~ End UVoxelToolBase Interface
};