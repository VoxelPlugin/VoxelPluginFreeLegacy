// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelValue.h"
#include "VoxelPaintMaterial.h"
#include "VoxelTools/VoxelToolsBase.h"
#include "VoxelSphereTools.generated.h"

class FVoxelData;
class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelSphereTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Bounds to lock
	inline static FIntBox GetSphereBounds(const FVoxelVector& Position, float Radius)
	{
		return FIntBox(Position - Radius - 3, Position + Radius + 3);
	}

	static void SetValueSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		FVoxelValue Value);

	template<bool bAdd, bool bComputeModifiedVoxels>
	static void SphereEditImpl(
		FVoxelData& Data, 
		const FVoxelVector& Position, 
		float Radius,
		TArray<FModifiedVoxelValue>& ModifiedVoxels);
	static void RemoveSphereImpl(
		FVoxelData& Data, 
		const FVoxelVector& Position, 
		float Radius,
		TArray<FModifiedVoxelValue>& ModifiedVoxels)
	{
		SphereEditImpl<false, true>(Data, Position, Radius, ModifiedVoxels);
	}
	static void RemoveSphereImpl(
		FVoxelData& Data, 
		const FVoxelVector& Position, 
		float Radius)
	{
		TArray<FModifiedVoxelValue> ModifiedVoxels;
		SphereEditImpl<false, false>(Data, Position, Radius, ModifiedVoxels);
	}
	static void AddSphereImpl(
		FVoxelData& Data, 
		const FVoxelVector& Position, 
		float Radius,
		TArray<FModifiedVoxelValue>& ModifiedVoxels)
	{
		SphereEditImpl<true, true>(Data, Position, Radius, ModifiedVoxels);
	}
	static void AddSphereImpl(
		FVoxelData& Data, 
		const FVoxelVector& Position, 
		float Radius)
	{
		TArray<FModifiedVoxelValue> ModifiedVoxels;
		SphereEditImpl<true, false>(Data, Position, Radius, ModifiedVoxels);
	}

	static void SetMaterialSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position, 
		float Radius, 
		const FVoxelPaintMaterial& PaintMaterial);
	
	static void ApplyKernelSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Center,
		float FirstDegreeNeighbor,
		float SecondDegreeNeighbor,
		float ThirdDegreeNeighbor);
	
	static void SmoothSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Strength);

	// "Sharpen" by lerping the voxels to -1 or +1 depending on their sign
	static void SharpenSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		float Strength);

	// Radius + Falloff must be locked
	static void TrimSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		const FVector& Normal,
		float Radius,
		float Falloff,
		bool bAdditive);

	static void RevertSphereImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues,
		bool bRevertMaterials);

	static void RevertSphereToGeneratorImpl(
		FVoxelData& Data,
		const FVoxelVector& Position,
		float Radius,
		bool bRevertValues,
		bool bRevertMaterials);

public:
	/**
	 * Set the density in a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Value					The density to set
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void SetValueSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Value,
		bool bConvertToVoxelSpace = true);

	/**
	 * Add a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void AddSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Remove a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void RemoveSphere(
		AVoxelWorld* World,
		FVector Position, 
		float Radius,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Paint material in a sphere shape
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	PaintMaterial			The material to paint
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void SetMaterialSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius, 
		FVoxelPaintMaterial PaintMaterial, 
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Apply a 3x3x3 kernel
	 * @param	World							The voxel world
	 * @param	Position						The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius							The radius, in cm if ConvertToVoxelSpace is true
	 * @param	CenterMultiplier				Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param   SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param   ThirdDegreeNeighborMultiplier	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	bConvertToVoxelSpace			If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void ApplyKernelSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Smooth a sphere
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Strength				The strength of the smoothing (preferably between 0 and 1)
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void SmoothSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Strength = 0.1f,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Sharpen a sphere
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Strength				The strength of the smoothing (preferably between 0 and 1)
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void SharpenSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Strength = 0.1f,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * 
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Normal					The normal
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Falloff					The falloff, in cm if ConvertToVoxelSpace is true
	 * @param	bAdditive				Whether to add or remove voxels
	 * @param	bConvertToVoxelSpace	If true, the position, radius and falloff will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void TrimSphere(
		AVoxelWorld* World,
		FVector Position,
		FVector Normal,
		float Radius,
		float Falloff,
		bool bAdditive = false,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	HistoryPosition			The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void RevertSphere(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues = true,
		bool bRevertMaterials = true,
		bool bConvertToVoxelSpace = true);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", AdvancedDisplay = "bConvertToVoxelSpace"))
	static void RevertSphereToGenerator(
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		bool bRevertValues = true,
		bool bRevertMaterials = true,
		bool bConvertToVoxelSpace = true);

public:
	/**
	 * Set the density in a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Value					The density to set
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void SetValueSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Value, 
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Add a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void AddSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius, 
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Remove a sphere
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void RemoveSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius, 
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Paint material in a sphere shape
	 * @param	World					The voxel world
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	PaintMaterial			The material to paint
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void SetMaterialSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		FVector Position,
		float Radius,
		FVoxelPaintMaterial PaintMaterial,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Apply a 3x3x3 kernel
	 * @param	World							The voxel world
	 * @param	Position						The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius							The radius, in cm if ConvertToVoxelSpace is true
	 * @param	CenterMultiplier				Multiplier of the center value
	 * @param	FirstDegreeNeighborMultiplier	Multiplier of the immediate neighbors, which share 2 coordinates with the center
	 * @param   SecondDegreeNeighborMultiplier	Multiplier of the near corners neighbors, which share 1 coordinates with the center
	 * @param   ThirdDegreeNeighborMultiplier	Multiplier of the far corners neighbors, which share 0 coordinates with the center
	 * @param	bConvertToVoxelSpace			If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void ApplyKernelSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float CenterMultiplier = 0.037f,
		float FirstDegreeNeighborMultiplier = 0.037f,
		float SecondDegreeNeighborMultiplier = 0.037f,
		float ThirdDegreeNeighborMultiplier = 0.037f,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Smooth a sphere asynchronously
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Strength				The strength of the smoothing (preferably between 0 and 1)
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void SmoothSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Strength = 0.1f,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Sharpen a sphere asynchronously
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Strength				The strength of the smoothing (preferably between 0 and 1)
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void SharpenSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		float Strength = 0.1f,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Trim tool: used to quickly flatten large portions of the world
	 * Best results are obtained when Position and Normal are averages: use FindProjectionVoxels to do some linetraces, and then GetAveragePosition/Normal on the result
	 * This ensures the tool usage is relatively smooth.
	 * 
	 * Works by stamping a shape into the world (if bAdditive = false, the stamp is destructive: voxels are removed instead)
	 * The shape is the smooth union of a sphere SDF and a plane SDF. The smoothness of the union is controlled by the Falloff parameter.
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Normal					The normal
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	Falloff					The falloff, in cm if ConvertToVoxelSpace is true
	 * @param	bAdditive				Whether to add or remove voxels
	 * @param	bConvertToVoxelSpace	If true, the position, radius and falloff will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void TrimSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		FVector Normal,
		float Radius,
		float Falloff,
		bool bAdditive = false,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Reverts the voxels inside a sphere shape to a previous frame in the undo history.
	 * Can be used to "paint" the undo history
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	HistoryPosition			The history position to go back to. You can use GetHistoryPosition to get it.
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void RevertSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		int32 HistoryPosition,
		bool bRevertValues = true,
		bool bRevertMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
	
	/**
	 * Reverts the voxels inside a sphere shape to their generator value
	 * @param	World					The voxel world		
	 * @param	Position				The position of the center, in world space if ConvertToVoxelSpace is true
	 * @param	Radius					The radius, in cm if ConvertToVoxelSpace is true
	 * @param	bRevertValues			Whether to revert values
	 * @param	bRevertMaterials		Whether to revert materials
	 * @param	bConvertToVoxelSpace	If true, the position and radius will be converted to voxel space. Else they will be used directly
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bConvertToVoxelSpace, bHideLatentWarnings"))
	static void RevertSphereToGeneratorAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		FVector Position,
		float Radius,
		bool bRevertValues = true,
		bool bRevertMaterials = true,
		bool bConvertToVoxelSpace = true,
		bool bHideLatentWarnings = false);
};