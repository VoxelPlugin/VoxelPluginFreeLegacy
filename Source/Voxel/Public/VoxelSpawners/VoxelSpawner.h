// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelGlobals.h"
#include "VoxelSpawner.generated.h"

class FVoxelConstDataAccelerator;
class FVoxelSpawnerManager;
class FVoxelSpawnerProxy;
class FVoxelData;
class AVoxelSpawnerActor;
class UVoxelSpawner;


UCLASS(Abstract)
class VOXEL_API UVoxelSpawner : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (ClampMin = 0))
	float DistanceBetweenInstancesInVoxel = 10;
	
public:
};