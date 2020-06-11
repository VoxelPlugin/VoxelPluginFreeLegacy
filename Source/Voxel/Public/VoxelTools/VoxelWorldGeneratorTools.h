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
	// Scale is applied to (Start + Position)
	template<typename T>
	static TVoxelTexture<T> CreateTextureFromWorldGeneratorImpl(
		const FVoxelWorldGeneratorInstance& WorldGenerator,
		FName OutputName,
		const FIntPoint& Start,
		const FIntPoint& Size,
		float Scale);
	
	/**
	 * Creates a float texture by reading a float output from a generator
	 * 
	 * @param	OutTexture			The result
	 * @param	WorldGenerator		The world generator to use
	 * @param	Seeds				The seeds to pass to the world generator, optional
	 * @param	OutputName			The output name to query. Must be a float output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 * @param	VoxelSize			The value returned by the Voxel Size node in the world generator graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", AdvancedDisplay = "StartX, StartY, VoxelSize"))
	static void CreateFloatTextureFromWorldGenerator(
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
	
	/**
	 * Creates a float texture by reading a float output from a generator, asynchronously
	 * 
	 * @param	OutTexture			The result
	 * @param	WorldGenerator		The world generator to use
	 * @param	Seeds				The seeds to pass to the world generator, optional
	 * @param	OutputName			The output name to query. Must be a float output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 * @param	VoxelSize			The value returned by the Voxel Size node in the world generator graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "StartX, StartY, VoxelSize, bHideLatentWarnings"))
	static void CreateFloatTextureFromWorldGeneratorAsync(
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

public:
	/**
	 * Creates a color texture by reading a color output from a generator
	 * 
	 * @param	OutTexture			The result
	 * @param	WorldGenerator		The world generator to use
	 * @param	Seeds				The seeds to pass to the world generator, optional
	 * @param	OutputName			The output name to query. Must be a color output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 * @param	VoxelSize			The value returned by the Voxel Size node in the world generator graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", AdvancedDisplay = "StartX, StartY, VoxelSize"))
	static void CreateColorTextureFromWorldGenerator(
		FVoxelColorTexture& OutTexture,
		UVoxelWorldGenerator* WorldGenerator,
		const TMap<FName, int32>& Seeds,
		FName OutputName = "MyColor",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		float VoxelSize = 100);
	
	/**
	 * Creates a color texture by reading a color output from a generator, asynchronously
	 * 
	 * @param	OutTexture			The result
	 * @param	WorldGenerator		The world generator to use
	 * @param	Seeds				The seeds to pass to the world generator, optional
	 * @param	OutputName			The output name to query. Must be a color output.
	 * @param	SizeX				The Size of the resulting texture on the X axis
	 * @param	SizeY				The Size of the resulting texture on the Y axis
	 * @param	Scale				Scale that can be used to scale the inputs: the generator will be queried as (Start + Position) * Scale
	 * @param	StartX				Where the texture starts
	 * @param	StartY				Where the texture starts
	 * @param	VoxelSize			The value returned by the Voxel Size node in the world generator graph
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|World Generators", meta = (AutoCreateRefTerm = "Seeds", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "StartX, StartY, VoxelSize, bHideLatentWarnings"))
	static void CreateColorTextureFromWorldGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelColorTexture& OutTexture,
		UVoxelWorldGenerator* WorldGenerator,
		const TMap<FName, int32>& Seeds,
		FName OutputName = "MyColor",
		int32 SizeX = 512,
		int32 SizeY = 512,
		float Scale = 1,
		int32 StartX = 0,
		int32 StartY = 0,
		float VoxelSize = 100,
		bool bHideLatentWarnings = false);
};