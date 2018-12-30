// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelMaterial.h"
#include "VoxelPaintMaterial.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Logging/MessageLog.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;

#define CHECK_WORLD_VOXELTOOLS(Name, ...) \
if (!World) \
{ \
	FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("%s: World is NULL"), TEXT(#Name)))); \
	return __VA_ARGS__; \
} \
 \
if (!World->IsCreated()) \
{ \
	FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("%s: World is no created"), TEXT(#Name)))); \
	return __VA_ARGS__; \
}

UCLASS()
class VOXEL_API UVoxelTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void AddNeighborsToSet(const TSet<FIntVector>& InSet, TSet<FIntVector>& OutSet);

	/**
	 * Set the density in a sphere
	 * @param	World		The voxel world
	 * @param	Position	The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius		The radius in voxels
	 * @param	Value		The density to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetValueSphere(   AVoxelWorld* World, FIntVector Position, float Radius, float Value);
	
	/**
	 * Add a sphere
	 * @param	World		The voxel world
	 * @param	Position	The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius		The radius in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void AddSphere(	      AVoxelWorld* World, FIntVector Position, float Radius);
	
	/**
	 * Remove a sphere
	 * @param	World		The voxel world
	 * @param	Position	The voxel position of the center (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Radius		The radius in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void RemoveSphere(     AVoxelWorld* World, FIntVector Position, float Radius);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetMaterialSphere(AVoxelWorld* World, FIntVector Position, float Radius, FVoxelPaintMaterial PaintMaterial, class UCurveFloat* StrengthCurve = nullptr);

public:
	/**
	 * Set the density in a box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetValueBox(   AVoxelWorld* World, FIntBox Bounds, float Value);
	
	/**
	 * Add a box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void AddBox(	       AVoxelWorld* World, FIntBox Bounds);
	
	/**
	 * Remove a box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void RemoveBox(	   AVoxelWorld* World, FIntBox Bounds);
	
	/**
	 * Paint a box
	 * @param	World			The voxel world
	 * @param	Position		The voxel position of the minimal corner (use GlobalToLocal function of the VoxelWorld to get it)
	 * @param	Size			The size of the box in voxels
	 * @param	PaintMaterial	The material to paint
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetMaterialBox(AVoxelWorld* World, FIntBox Bounds, FVoxelPaintMaterial PaintMaterial);
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void SetBoxAsDirty(AVoxelWorld* World, FIntBox Bounds, bool bSetValuesAsDirty = true, bool bSetMaterialsAsDirty = true);

	/**
	 * Round voxels that don't have an impact on the surface. Same visual result but will lead to better compression
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void RoundVoxels(AVoxelWorld* World, FIntBox Bounds);

public:
	/**
	 * Flatten the surface of the world
	 * @param	World								Voxel World
	 * @param	Position							Position in world space
	 * @param	Normal								Normal at Position
	 * @param	Radius								Radius in world space
	 * @param	Strength							Speed of the edit
	 * @param	bDontModifyVoxelsAroundPosition		Don't edit the voxels around Position. Use if the tool is moving when editing
	 * @param	bDontModifyEmptyVoxels				Can't add on voxels totally empty
	 * @param	bDontModifyFullVoxels				Can't remove on voxels totally full
	 * @param	bShowModifiedVoxels					Draw debug points on modified voxels
	 * @param	bShowTestedVoxels					Draw debug points on voxels tested for validity
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "4"))
	static bool Flatten(AVoxelWorld* World, FVector Position, FVector Normal, float Radius, float Strength = 0.1, bool bDontModifyVoxelsAroundPosition = false, bool bDontModifyEmptyVoxels = false, bool bDontModifyFullVoxels = false, int TimeoutInMicroSeconds = 500, bool bShowModifiedVoxels = false, bool bShowTestedVoxels = false);
};
