// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LatentActionManager.h"
#include "VoxelTexture.h"
#include "VoxelWorldGeneratorTools.generated.h"

class FVoxelWorldGeneratorInstance;
class UTexture2D;
class UVoxelWorldGenerator;
class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelWorldGeneratorTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TVoxelTexture<float> CreateTextureFromWorldGeneratorImpl(
		const FVoxelWorldGeneratorInstance& WorldGenerator,
		FName OutputName,
		const FIntPoint& Start,
		const FIntPoint& Size,
		float Scale);
	
	// Voxel Size: will be passed to the generator
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", AdvancedDisplay = "StartX, StartY, VoxelSize"))
	static void CreateTextureFromWorldGenerator(
		FVoxelFloatTexture& OutTexture,
		UVoxelWorldGenerator* WorldGenerator,
		const TMap<FName, int32>& Seeds,
		FName OutputName = "Value",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		float VoxelSize = 100);

	// Voxel Size: will be passed to the generator
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "StartX, StartY, VoxelSize, bHideLatentWarnings"))
	static void CreateTextureFromWorldGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelFloatTexture& OutTexture,
		UVoxelWorldGenerator* WorldGenerator,
		const TMap<FName, int32>& Seeds,
		FName OutputName = "Value",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		float VoxelSize = 100,
		bool bHideLatentWarnings = false);
};