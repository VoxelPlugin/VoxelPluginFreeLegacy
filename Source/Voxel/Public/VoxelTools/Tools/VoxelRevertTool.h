// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/Tools/VoxelSphereToolBase.h"
#include "VoxelRevertTool.generated.h"

UCLASS()
class VOXEL_API UVoxelRevertTool : public UVoxelSphereToolBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bRevertValues = true;
	
	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	bool bRevertMaterials = false;

	UPROPERTY(Category = "Tool Settings", EditAnywhere, BlueprintReadWrite)
	int32 HistoryPosition = 0;

	UPROPERTY(Category = "Tool Settings", VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentHistoryPosition = 0;

public:
	UVoxelRevertTool();
	
	//~ Begin UVoxelToolBase Interface
	virtual void Tick() override;
	virtual FVoxelIntBoxWithValidity DoEdit() override;
	//~ End UVoxelToolBase Interface
};