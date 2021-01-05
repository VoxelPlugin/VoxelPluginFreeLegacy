// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelCubicLibrary.generated.h"

class AVoxelWorld;

UCLASS()
class UVoxelCubicLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Use bSelectVoxelOutside to choose which voxel to select, between the two sides of the face hit 
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Cubic", meta = (DefaultToSelf = "World"))
	static FIntVector GetCubicVoxelPositionFromHit(AVoxelWorld* World, FVector HitPosition, FVector HitNormal, bool bSelectVoxelOutside = false);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Cubic", meta = (DefaultToSelf = "World"))
	static bool GetCubicVoxelValue(AVoxelWorld* World, FIntVector Position);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Cubic", meta = (DefaultToSelf = "World"))
	static void SetCubicVoxelValue(AVoxelWorld* World, FIntVector Position, bool bValue);
};