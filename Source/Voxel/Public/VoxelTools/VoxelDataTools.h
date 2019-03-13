// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPaintMaterial.h"
#include "VoxelData/VoxelSave.h"
#include "VoxelDataTools.generated.h"

class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelDataTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Keywords = "density", AdvancedDisplay = "bAllowFailure"))
	static bool GetValue(
		float& Value,
		AVoxelWorld* World, 
		const FIntVector& Position,
		bool bAllowFailure = false);
	/**
	 * Set the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Value			Density to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Keywords = "density", AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetValue(
		AVoxelWorld* World, 
		const FIntVector& Position, 
		float Value, 
		bool bUpdateRender = true,
		bool bAllowFailure = false);
	
	/**
	 * Get the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (AdvancedDisplay = "bAllowFailure"))
	static bool GetMaterial(
		FVoxelMaterial& Material,
		AVoxelWorld* World, 
		const FIntVector& Position,
		bool bAllowFailure = false);
	/**
	 * Set the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Material		Material to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetMaterial(
		AVoxelWorld* World, 
		const FIntVector& Position, 
		FVoxelMaterial Material, 
		bool bUpdateRender = true,
		bool bAllowFailure = false);

public:
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density"))
	static void GetValueAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		float& Value,
		AVoxelWorld* World, 
		const FIntVector& Position);
	/**
	 * Set the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Value			Density to set
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density", AdvancedDisplay = "bUpdateRender"))
	static void SetValueAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FIntVector& Position, 
		float Value, 
		bool bUpdateRender = true);
	
	/**
	 * Get the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject"))
	static void GetMaterialAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelMaterial& Material,
		AVoxelWorld* World, 
		const FIntVector& Position);
	/**
	 * Set the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Material		Material to set
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void SetMaterialAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FIntVector& Position, 
		FVoxelMaterial Material, 
		bool bUpdateRender = true);

public:
	/**
	 * Get a save of the world
	 * @param	World		The voxel world
	 * @param	OutSave		The save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void GetSave(
		AVoxelWorld* World, 
		FVoxelUncompressedWorldSave& OutSave);
	/**
	 * Get a save of the world and compress it
	 * @param	World		The voxel world
	 * @param	OutSave		The compressed save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void GetCompressedSave(
		AVoxelWorld* World, 
		FVoxelCompressedWorldSave& OutSave);
	
	/**
	 * Load from a save
	 * @param	World			The voxel world
	 * @param	Save			The save to load from
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (AdvancedDisplay = "bUpdateRender"))
	static void LoadFromSave(
		AVoxelWorld* World, 
		const FVoxelUncompressedWorldSave& Save, 
		bool bUpdateRender = true);
	/**
	 * Load from a compressed save
	 * @param	World			The voxel world
	 * @param	Save			The compressed save to load from
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (AdvancedDisplay = "bUpdateRender"))
	static void LoadFromCompressedSave(
		AVoxelWorld* World, 
		const FVoxelCompressedWorldSave& Save, 
		bool bUpdateRender = true);

public:
	/**
	 * Get a save of the world
	 * @param	World		The voxel world
	 * @param	OutSave		The save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject"))
	static void GetSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelUncompressedWorldSave& OutSave);
	/**
	 * Get a save of the world and compress it
	 * @param	World		The voxel world
	 * @param	OutSave		The compressed save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject"))
	static void GetCompressedSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelCompressedWorldSave& OutSave);
	
	/**
	 * Load from a save
	 * @param	World			The voxel world
	 * @param	Save			The save to load from
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void LoadFromSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FVoxelUncompressedWorldSave& Save, 
		bool bUpdateRender = true);
	/**
	 * Load from a compressed save
	 * @param	World			The voxel world
	 * @param	Save			The compressed save to load from
	 * @param	bUpdateRender	Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void LoadFromCompressedSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FVoxelCompressedWorldSave& Save, 
		bool bUpdateRender = true);
};