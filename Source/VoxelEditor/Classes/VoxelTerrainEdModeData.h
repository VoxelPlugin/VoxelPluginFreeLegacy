// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools.h"
#include "VoxelTerrainEdModeData.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelTerrainEdModeData : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(Category = "Tool Settings", EditAnywhere, meta = (ShowForTools = "ProjectionTool,SphereTool,BoxTool", ClampMin = "0", UIMin = "0", UIMax = "1000"))
	float Radius = 500.f;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, meta = (ShowForTools = "ProjectionTool", ShowForModes = "EditMode", UIMin = "0", UIMax = "1"))
	float Strength = 0.3f;

	UPROPERTY(Category = "Mode Settings", EditAnywhere, meta = (ShowForModes = "PaintMode"))
	uint8 MaterialIndex;
	UPROPERTY(Category = "Mode Settings", EditAnywhere, meta = (ShowForModes = "PaintMode"))
	EVoxelLayer Layer;

	UPROPERTY(Category = "Controls", EditAnywhere)
	FKey Add = EKeys::LeftControl;
};