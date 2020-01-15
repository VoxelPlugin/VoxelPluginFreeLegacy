// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LatentActionManager.h"
#include "VoxelTexture.h"
#include "VoxelWorldGeneratorTools.generated.h"

class UTexture2D;
class UVoxelWorldGenerator;
class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelWorldGeneratorTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CreateTextureFromWorldGenerator(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelFloatTexture& OutTexture,
		UVoxelWorldGenerator* WorldGenerator,
		const TMap<FName, int32>& Seeds,
		FName OutputName = "Value",
		float VoxelSize = 100,
		int32 StartX = 0,
		int32 StartY = 0,
		int32 SizeX = 512,
		int32 SizeY = 512,
		bool bHideLatentWarnings = false);
};