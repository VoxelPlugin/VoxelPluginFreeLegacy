// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelDebugUtilities.generated.h"

class AVoxelWorld;

UCLASS()
class UVoxelDebugUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel|Debug", meta = (AdvancedDisplay = "BorderOffset"))
	static void DrawDebugIntBox(AVoxelWorld* World, const FIntBox& Box, float Lifetime = 1, float Thickness = 0, FLinearColor Color = FLinearColor::Red);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel|Debug")
	static void DebugVoxelsInsideBounds(
		AVoxelWorld* World,
		FIntBox Bounds, 
		FLinearColor Color = FLinearColor::Red, 
		float Lifetime = 1, 
		float Thickness = 1, 
		bool bDebugDensities = true, 
		FLinearColor TextColor = FLinearColor::Black);
};