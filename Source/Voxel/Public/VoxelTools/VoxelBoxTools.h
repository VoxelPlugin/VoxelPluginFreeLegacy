// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPaintMaterial.h"
#include "IntBox.h"
#include "VoxelBoxTools.generated.h"

class AVoxelWorld;
struct FIntBox;

UCLASS()
class VOXEL_API UVoxelBoxTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Set the density in a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	Value			The value to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetValueBox(
		AVoxelWorld* World,
		const FIntBox& Bounds,
		float Value,
		bool bUpdateRender = true,
		bool bAllowFailure = false);

	/**
	 * Add a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool AddBox(
		AVoxelWorld* World,
		const FIntBox& Bounds,
		bool bUpdateRender = true,
		bool bAllowFailure = false);

	/**
	 * Remove a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool RemoveBox(
		AVoxelWorld* World,
		const FIntBox& Bounds,
		bool bUpdateRender = true,
		bool bAllowFailure = false);

	/**
	 * Paint a box shape
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	PaintMaterial	The material to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetMaterialBox(
		AVoxelWorld* World,
		const FIntBox& Bounds,
		FVoxelPaintMaterial PaintMaterial,
		bool bUpdateRender = true,
		bool bAllowFailure = false);

public:
	/**
	 * Set the density in a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	Value			The value to set
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void SetValueBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntBox& Bounds,
		float Value,
		bool bUpdateRender = true);

	/**
	 * Add a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void AddBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntBox& Bounds,
		bool bUpdateRender = true);

	/**
	 * Remove a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void RemoveBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntBox& Bounds,
		bool bUpdateRender = true);

	/**
	 * Paint a box shape async
	 * @param	World			The voxel world
	 * @param	Bounds			The bounds of the box
	 * @param	PaintMaterial	The material to set
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Box Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void SetMaterialBoxAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntBox& Bounds,
		FVoxelPaintMaterial PaintMaterial,
		bool bUpdateRender = true);
};