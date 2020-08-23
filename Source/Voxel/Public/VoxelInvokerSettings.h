// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelInvokerSettings.generated.h"

USTRUCT(BlueprintType)
struct FVoxelInvokerSettings
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	bool bUseForLOD = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	int32 LODToSet = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	FVoxelIntBox LODBounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	bool bUseForCollisions = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	FVoxelIntBox CollisionsBounds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	bool bUseForNavmesh = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Voxel")
	FVoxelIntBox NavmeshBounds;
};