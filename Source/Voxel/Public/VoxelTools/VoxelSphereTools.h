// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPaintMaterial.h"
#include "VoxelSphereTools.generated.h"

class AVoxelWorld;
class UCurveFloat;

UCLASS()
class VOXEL_API UVoxelSphereTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Set the density in a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	Value			The density to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetValueSphere(
		AVoxelWorld* World,
		const FIntVector& Position, 
		float Radius,
		float Value, 
		bool bUpdateRender = true,
		bool bAllowFailure = false);
	
	/**
	 * Add a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool AddSphere(
		AVoxelWorld* World,
		const FIntVector& Position,
		float Radius,
		bool bUpdateRender = true,
		bool bAllowFailure = false);
	
	/**
	 * Remove a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool RemoveSphere(
		AVoxelWorld* World,
		const FIntVector& Position, 
		float Radius,
		bool bUpdateRender = true,
		bool bAllowFailure = false);
	
	/**
	 * Paint material in a sphere shape
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	PaintMaterial	The material to paint
	 * @param	StrengthCurve	Optional. Curve X axis is the distance to the center normalized (between 0 and 1), and Y axis is the paint strength (between 0 and 1)
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (AdvancedDisplay = "bUpdateRender, bAllowFailure"))
	static bool SetMaterialSphere(
		AVoxelWorld* World,
		const FIntVector& Position,
		float Radius, 
		const FVoxelPaintMaterial& PaintMaterial, 
		UCurveFloat* StrengthCurve = nullptr, 
		bool bUpdateRender = true,
		bool bAllowFailure = false);

public:
	/**
	 * Set the density in a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	Value			The density to set
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void SetValueSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntVector& Position,
		float Radius,
		float Value, 
		bool bUpdateRender = true);
	
	/**
	 * Add a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void AddSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntVector& Position,
		float Radius, 
		bool bUpdateRender = true);
	
	/**
	 * Remove a sphere
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void RemoveSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World,
		const FIntVector& Position,
		float Radius, 
		bool bUpdateRender = true);
	
	/**
	 * Paint material in a sphere shape
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius			The radius in voxels
	 * @param	PaintMaterial	The material to paint
	 * @param	StrengthCurve	Optional. Curve X axis is the distance to the center normalized (between 0 and 1), and Y axis is the paint strength (between 0 and 1)
	 * @param	bUpdateRender	Should the render be updated?
	 * @param   bAllowFailure	If the data is locked by another thread, fail instead of waiting
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Sphere Tools", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bUpdateRender"))
	static void SetMaterialSphereAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FIntVector& Position,
		float Radius,
		const FVoxelPaintMaterial& PaintMaterial,
		UCurveFloat* StrengthCurve = nullptr,
		bool bUpdateRender = true);

};