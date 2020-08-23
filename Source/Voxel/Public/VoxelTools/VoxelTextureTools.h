// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTexture.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTextureTools.generated.h"

UCLASS()
class VOXEL_API UVoxelTextureTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Apply the photoshop Minimum filter
	// Set each pixel to the min value in a radius Radius
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static FVoxelFloatTexture Minimum(
		FVoxelFloatTexture Texture,
		float Radius = 2);
	
	// Apply the photoshop Maximum filter
	// Set each pixel to the max value in a radius Radius
	UFUNCTION(BlueprintCallable, Category = "Voxel|Voxel Texture")
	static FVoxelFloatTexture Maximum(
		FVoxelFloatTexture Texture,
		float Radius = 2);
};