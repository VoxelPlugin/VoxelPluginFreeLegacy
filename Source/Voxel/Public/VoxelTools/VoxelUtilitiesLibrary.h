// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IntBox.h"
#include "VoxelUtilitiesLibrary.generated.h"

struct FIntBox;
class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelUtilitiesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Utilities")

	static void AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Utilities")
	static void SetBoxAsDirty(AVoxelWorld* World, const FIntBox& Bounds, bool bSetValuesAsDirty = true, bool bSetMaterialsAsDirty = true);

	// Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	UFUNCTION(BlueprintCallable, Category = "Voxel|Utilities")
	static void RoundVoxels(AVoxelWorld* World, const FIntBox& Bounds);
};