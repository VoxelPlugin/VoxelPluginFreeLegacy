// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelValue.h"
#include "VoxelPaintMaterial.h"
#include "VoxelIntBox.h"
#include "VoxelBoxTools.generated.h"

class FVoxelData;
class AVoxelWorld;
struct FVoxelIntBox;

UCLASS()
class VOXEL_API UVoxelBoxTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void SetValueBoxImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, FVoxelValue Value);

	template<bool bAdd>
	static void BoxEditImpl(FVoxelData& Data, const FVoxelIntBox& Bounds);

	static void SetMaterialBoxImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, const FVoxelPaintMaterial& PaintMaterial);

public:
	/**
	 * Set the density in a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	Value			The value to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World"))
	static void SetValueBox(AVoxelWorld* World,	FVoxelIntBox Bounds,	float Value);

	/**
	 * Add a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World"))
	static void AddBox(AVoxelWorld* World, FVoxelIntBox Bounds);

	/**
	 * Remove a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World"))
	static void RemoveBox(AVoxelWorld* World, FVoxelIntBox Bounds);

	/**
	 * Paint a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	PaintMaterial	The material to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World"))
	static void SetMaterialBox(AVoxelWorld* World, FVoxelIntBox Bounds, FVoxelPaintMaterial PaintMaterial);

public:
	/**
	 * Set the density in a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	Value			The value to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void SetValueBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		float Value,
		bool bHideLatentWarnings = false);

	/**
	 * Add a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void AddBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);

	/**
	 * Remove a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void RemoveBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);

	/**
	 * Paint a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	PaintMaterial	The material to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void SetMaterialBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		FVoxelPaintMaterial PaintMaterial,
		bool bHideLatentWarnings = false);
};