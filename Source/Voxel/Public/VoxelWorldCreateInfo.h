// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelSave.h"
#include "VoxelWorldCreateInfo.generated.h"

class AVoxelWorld;
class FVoxelData;

USTRUCT(BlueprintType)
struct FVoxelWorldCreateInfo
{
	GENERATED_BODY()
		
public:
	// If OverrideSave is true, the world will load SaveOverride instead of the save object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bOverrideSave = false;

	// If OverrideSave is true, the world will load SaveOverride instead of the save object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelUncompressedWorldSave SaveOverride;

public:
	// If bOverrideData is true, will use DataSource data instead of creating a new data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bOverrideData = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	AVoxelWorld* DataOverride = nullptr;
	
	TVoxelSharedPtr<FVoxelData> DataOverride_Raw;
};