// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelData/VoxelSave.h"
// TODO REMOVE
#include "VoxelData/VoxelDataLock.h"
#include "VoxelDataTools.generated.h"

class FVoxelData;
class AVoxelWorld;
class UVoxelGenerator;
class UVoxelHeightmapAsset;
template<typename T>
struct TVoxelHeightmapAssetSamplerWrapper;

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

USTRUCT(BlueprintType)
struct FVoxelDataMemoryUsageInMB
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float DirtyValues = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float CachedValues = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float DirtyMaterials = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float CachedMaterials = 0;
};

USTRUCT(BlueprintType)
struct FVoxelFindClosestNonEmptyVoxelResult
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bSuccess = false;
	
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density", DisplayName = "Get Density"))
	static void GetValue(
		float& Value,
		AVoxelWorld* World, 
		FIntVector Position);
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density", DisplayName = "Get Interpolated Density"))
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Keywords = "density", DisplayName = "Set Density"))
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
	static void CacheValues(AVoxelWorld* World, FVoxelIntBox Bounds, bool bMultiThreaded = true);
	// Cache the materials in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World"))
	static void CacheMaterials(AVoxelWorld* World, FVoxelIntBox Bounds, bool bMultiThreaded = true);

public:
	/**
	 * Get the density at Position
	 * @param	World			The voxel world
	 * @param	Position		The voxel position (use the World Position to Voxel function of the VoxelWorld to get it)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density", AdvancedDisplay = "bHideLatentWarnings", DisplayName = "Get Density Async"))
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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", Keywords = "density", AdvancedDisplay = "bHideLatentWarnings", DisplayName = "Set Density Async"))
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
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);
	// Cache the materials in the bounds
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CacheMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
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
	static void GetSave(
		AVoxelWorld* World, 
		FVoxelUncompressedWorldSaveImpl& OutSave, 
		TArray<FVoxelObjectArchiveEntry>& OutObjects);
	/**
	 * Get a save of the world and compress it
	 * @param	World		The voxel world
	 * @param	OutSave		The compressed save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static void GetCompressedSave(
		AVoxelWorld* World, 
		FVoxelCompressedWorldSave& OutSave);
	static void GetCompressedSave(
		AVoxelWorld* World, 
		FVoxelCompressedWorldSaveImpl& OutSave, 
		TArray<FVoxelObjectArchiveEntry>& OutObjects);
	/**
	 * Get a save of the world
	 * @param	World		The voxel world
	 * @param	OutSave		The save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void GetSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelUncompressedWorldSave& OutSave,
		bool bHideLatentWarnings = false);
	/**
	 * Get a save of the world and compress it
	 * @param	World		The voxel world
	 * @param	OutSave		The compressed save	
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void GetCompressedSaveAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelCompressedWorldSave& OutSave,
		bool bHideLatentWarnings = false);
	
	/**
	 * Load from a save
	 * @param	World			The voxel world
	 * @param	Save			The save to load from
	 * @return	If the load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static bool LoadFromSave(
		const AVoxelWorld* World, 
		const FVoxelUncompressedWorldSave& Save);
	static bool LoadFromSave(
		const AVoxelWorld* World, 
		const FVoxelUncompressedWorldSaveImpl& Save, 
		const TArray<FVoxelObjectArchiveEntry>& Objects);
	
	/**
	 * Load from a compressed save
	 * @param	World			The voxel world
	 * @param	Save			The compressed save to load from
	 * @return	If the load was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World"))
	static bool LoadFromCompressedSave(
		const AVoxelWorld* World, 
		const FVoxelCompressedWorldSave& Save);
	static bool LoadFromCompressedSave(
		const AVoxelWorld* World, 
		const FVoxelCompressedWorldSaveImpl& Save, 
		const TArray<FVoxelObjectArchiveEntry>& Objects);

public:
	// Bounds.Extend(2) must be locked!
	// Bounds can be FVoxelIntBox::Infinite
	static void RoundVoxelsImpl(FVoxelData& Data, const FVoxelIntBox& Bounds);
	
	// Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void RoundVoxels(AVoxelWorld* World, FVoxelIntBox Bounds);
	
	// Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void RoundVoxelsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	// Bounds.Extend(1) must be locked!
	// Bounds can be FVoxelIntBox::Infinite
	static void ClearUnusedMaterialsImpl(FVoxelData& Data, const FVoxelIntBox& Bounds);
	
	// Remove materials that do not affect the surface. Same visual result but will lead to better compression.
	// Digging will look different.
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data")
	static void ClearUnusedMaterials(AVoxelWorld* World, FVoxelIntBox Bounds);
	
	// Remove materials that do not affect the surface. Same visual result but will lead to better compression.
	// Digging will look different.
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void ClearUnusedMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	static void GetVoxelsValueAndMaterialImpl(
		FVoxelData& Data,
		TArray<FVoxelValueMaterial>& Voxels,
		const FVoxelIntBox& Bounds,
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

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Memory", meta = (DefaultToSelf = "World"))
	static FVoxelDataMemoryUsageInMB GetDataMemoryUsageInMB(AVoxelWorld* World);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World"))
	static void ClearCachedValues(AVoxelWorld* World, FVoxelIntBox Bounds);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void ClearCachedValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World"))
	static void ClearCachedMaterials(AVoxelWorld* World, FVoxelIntBox Bounds);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Cache", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void ClearCachedMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void CheckForSingleValues(AVoxelWorld* World, FVoxelIntBox Bounds);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CheckForSingleValuesAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void CheckForSingleMaterials(AVoxelWorld* World, FVoxelIntBox Bounds);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CheckForSingleMaterialsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds,
		bool bHideLatentWarnings = false);

public:
	// Requires full write lock
	template<typename T>
	static void CompressIntoHeightmapImpl(FVoxelData& Data, TVoxelHeightmapAssetSamplerWrapper<T>& Wrapper, bool bCheckAllLeaves);

	/**
	 * If the voxel generator is a heightmap or if an heightmap asset is provided,
	 * will update the heightmap to the max Z surface in the voxel world
	 * Will not edit the data: RoundToGenerator should be called after for best results
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void CompressIntoHeightmap(
		AVoxelWorld* World, 
		UVoxelHeightmapAsset* HeightmapAsset = nullptr,
		bool bHeightmapAssetMatchesWorld = false);

public:
	// Bounds.Extend(1) needs to be locked to read the additional densities!
	template<typename T1, typename T2, typename T3>
	static void MergeDistanceFieldImpl(
		FVoxelData& Data,
		const FVoxelIntBox& Bounds,
		T1 GetSDF,
		T2 MergeSDF,
		bool bMultiThreaded,
		bool bSetMaterials = false,
		T3 GetMaterial = [](float OldValue, float NewValue, FVoxelMaterial PreviousMaterial) { return PreviousMaterial; });

public:
	// Requires write lock.
	static void RoundToGeneratorImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, bool bPreserveNormals);
	
	// Will revert the values who don't have a voxel neighbor with a different sign from the generator value
	// Will ignore items when computing generator values
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void RoundToGenerator(AVoxelWorld* World, FVoxelIntBox Bounds, bool bPreserveNormals = true);
	
	// Will revert the values who don't have a voxel neighbor with a different sign from the generator value
	// Will ignore items when computing generator values
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void RoundToGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds, 
		bool bPreserveNormals = true,
		bool bHideLatentWarnings = false);
	
public:
	// Requires write lock.
	static void CheckIfSameAsGeneratorImpl(FVoxelData& Data, const FVoxelIntBox& Bounds);
	
	// Will undirty the chunks identical to the generator
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void CheckIfSameAsGenerator(AVoxelWorld* World, FVoxelIntBox Bounds);

	// Will undirty the chunks identical to the generator
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void CheckIfSameAsGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds, 
		bool bHideLatentWarnings = false);

public:
	template<typename T>
	static void SetBoxAsDirtyImpl(FVoxelData& Data, const FVoxelIntBox& Bounds, bool bCompress);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World"))
	static void SetBoxAsDirty(AVoxelWorld* World, FVoxelIntBox Bounds, bool bDirtyValues = true, bool bDirtyMaterials = true);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static void SetBoxAsDirtyAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVoxelIntBox Bounds, 
		bool bDirtyValues = true, 
		bool bDirtyMaterials = true,
		bool bHideLatentWarnings = false);

public:
	// All neighbors of Position need to be locked (ie, FVoxelIntBox(Position, Position + 1))
	static FVoxelFindClosestNonEmptyVoxelResult FindClosestNonEmptyVoxelImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		bool bReadMaterial);
	
	/**
	 * Finds the closest voxel to Position that is not empty
	 * This is useful to do edits, or to query the material at a position
	 *
	 * @param		World					The voxel world
	 * @param		Position				The position, in world space if bConvertToVoxelSpace is true
	 * @param		bReadMaterial			If false will not read the material
	 * @param		bConvertToVoxelSpace	If true, will convert Position to voxel space. If false, assumes it's already in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", AdvancedDisplay = "bReadMaterial, bConvertToVoxelSpace"))
	static void FindClosestNonEmptyVoxel(
		FVoxelFindClosestNonEmptyVoxelResult& Result,
		AVoxelWorld* World,
		FVector Position,
		bool bReadMaterial = true,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Finds the closest voxel to Position that is not empty
	 * This is useful to do edits, or to query the material at a position
	 *
	 * @param		World					The voxel world
	 * @param		Position				The position, in world space if bConvertToVoxelSpace is true
	 * @param		bReadMaterial			If false will not read the material
	 * @param		bConvertToVoxelSpace	If true, will convert Position to voxel space. If false, assumes it's already in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Data", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bReadMaterial, bConvertToVoxelSpace, bHideLatentWarnings"))
	static void FindClosestNonEmptyVoxelAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		FVoxelFindClosestNonEmptyVoxelResult& Result,
		AVoxelWorld* World,
		FVector Position,
		bool bReadMaterial = true,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
};