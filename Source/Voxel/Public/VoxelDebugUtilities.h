// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelDebugUtilities.generated.h"

class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelDebugUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel|Debug", meta = (DefaultToSelf = "World", AdvancedDisplay = "Transform"))
	static void DrawDebugIntBox(
		AVoxelWorld* World, 
		FIntBox Bounds,
		FTransform Transform, 
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red);

	static void DrawDebugIntBox(
		AVoxelWorld* World,
		FIntBox Box,
		float Lifetime = 1,
		float Thickness = 0,
		FLinearColor Color = FLinearColor::Red)
	{
		DrawDebugIntBox(World, Box, FTransform(), Lifetime, Thickness, Color);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel|Debug", meta = (DefaultToSelf = "World"))
	static void DebugVoxelsInsideBounds(
		AVoxelWorld* World,
		FIntBox Bounds, 
		FLinearColor Color = FLinearColor::Red, 
		float Lifetime = 1, 
		float Thickness = 1, 
		bool bDebugDensities = true, 
		FLinearColor TextColor = FLinearColor::Black);
};