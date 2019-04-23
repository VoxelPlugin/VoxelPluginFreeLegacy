// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelMaterial.h"
#include "VoxelTools/VoxelPaintMaterial.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelTools.generated.h"

class AVoxelWorld;
class UCurveFloat;

UCLASS()
class VOXEL_API UVoxelTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

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
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools", meta = (AdvancedDisplay = "4"))
	static bool Flatten(
		AVoxelWorld* World,
		FVector Position,
		FVector Normal, 
		float Radius, 
		float Strength = 0.1, 
		bool bDontModifyVoxelsAroundPosition = false,
		bool bDontModifyEmptyVoxels = false,
		bool bDontModifyFullVoxels = false, 
		float LockTimeoutInSeconds = 0.0001,
		bool bShowModifiedVoxels = false,
		bool bShowTestedVoxels = false);
};
