// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelData/VoxelSave.h"
#include "VoxelDataTools.generated.h"

class FVoxelData;
class AVoxelWorld;
class UVoxelWorldGenerator;

USTRUCT(BlueprintType)
struct FVoxelValueMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelMaterial Material = FVoxelMaterial(ForceInit);
};

UCLASS()
class VOXEL_API UVoxelDataTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density"))
	static void GetValue(
		float& Value,
		AVoxelWorld* World, 
		FIntVector Position);
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density"))
	static void GetInterpolatedValue(
		float& Value,
		AVoxelWorld* World, 
		FVector Position);
	/**
	 * Set the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 * @param	Value			Density to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density"))
	static void SetValue(
		AVoxelWorld* World, 
		FIntVector Position, 
		float Value);
	
	/**
	 * Get the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void GetMaterial(
		FVoxelMaterial& Material,
		AVoxelWorld* World, 
		FIntVector Position);
	/**
	 * Set the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 * @param	Material		Material to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void SetMaterial(
		AVoxelWorld* World, 
		FIntVector Position, 
		FVoxelMaterial Material);

	// Cache the values in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World"))
	static void CacheValues(AVoxelWorld* World, FIntBox Bounds);
	// Cache the materials in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World"))
	static void CacheMaterials(AVoxelWorld* World, FIntBox Bounds);

public:
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density", AdvancedDisplay = "bHideLatentWarnings"))
	static void GetValueAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		float& Value,
		AVoxelWorld* World, 
		FIntVector Position,
		bool bHideLatentWarnings = false);
	/**
	 * Set the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 * @param	Value			Density to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density", AdvancedDisplay = "bHideLatentWarnings"))
	static void SetValueAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FIntVector Position, 
		float Value,
		bool bHideLatentWarnings = false);
	
	/**
	 * Get the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void GetMaterialAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelMaterial& Material,
		AVoxelWorld* World, 
		FIntVector Position,
		bool bHideLatentWarnings = false);
	/**
	 * Set the material at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 * @param	Material		Material to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void SetMaterialAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FIntVector Position, 
		FVoxelMaterial Material, 
		bool bHideLatentWarnings = false);

	// Cache the values in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CacheValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FIntBox Bounds,
		bool bHideLatentWarnings = false);
	// Cache the materials in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CacheMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	/**
	 * Get a save of the world
	 * @param	World		The voxel world
	 * @param	OutSave		The save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void GetSave(
		AVoxelWorld* World, 
		FVoxelUncompressedWorldSave& OutSave);
	/**
	 * Get a save of the world and compress it
	 * @param	World		The voxel world
	 * @param	OutSave		The compressed save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void GetCompressedSave(
		AVoxelWorld* World, 
		FVoxelCompressedWorldSave& OutSave);
	
	/**
	 * Load from a save
	 * @param	World			The voxel world
	 * @param	Save			The save to load from
	 * @return	If the load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static bool LoadFromSave(
		AVoxelWorld* World, 
		const FVoxelUncompressedWorldSave& Save);
	/**
	 * Load from a compressed save
	 * @param	World			The voxel world
	 * @param	Save			The compressed save to load from
	 * @return	If the load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static bool LoadFromCompressedSave(
		AVoxelWorld* World, 
		const FVoxelCompressedWorldSave& Save);

public:
	// Bounds.Extend(2) must be locked!
	// Bounds can be FIntBox::Infinite
	static void RoundVoxelsImpl(FVoxelData& Data, const FIntBox& Bounds);
	
	// Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void RoundVoxels(AVoxelWorld* World, FIntBox Bounds);
	
	// Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void RoundVoxelsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	// Bounds.Extend(1) must be locked!
	// Bounds can be FIntBox::Infinite
	static void ClearUnusedMaterialsImpl(FVoxelData& Data, const FIntBox& Bounds);
	
	// Remove materials that do not affect the surface. Same visual result but will lead to better compression.
	// Digging will look different.
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void ClearUnusedMaterials(AVoxelWorld* World, FIntBox Bounds);
	
	// Remove materials that do not affect the surface. Same visual result but will lead to better compression.
	// Digging will look different.
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void ClearUnusedMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	static void GetVoxelsValueAndMaterialImpl(
		FVoxelData& Data,
		TArray<FVoxelValueMaterial>& Voxels,
		const FIntBox& Bounds,
		const TArray<FIntVector>& Positions);

	// Read a large number of voxels at a time
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void GetVoxelsValueAndMaterial(
		TArray<FVoxelValueMaterial>& Voxels,
		AVoxelWorld* World,
		const TArray<FIntVector>& Positions);
	
	// Read a large number of voxels at a time, asynchronously
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void GetVoxelsValueAndMaterialAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FVoxelValueMaterial>& Voxels,
		AVoxelWorld* World,
		const TArray<FIntVector>& Positions,
		bool bHideLatentWarnings = false);
};