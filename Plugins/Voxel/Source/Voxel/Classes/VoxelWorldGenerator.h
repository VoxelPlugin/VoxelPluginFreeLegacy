// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VoxelWorldGenerator.generated.h"

class AVoxelWorld;

/**
 *
 */
UINTERFACE(BlueprintType)
class VOXEL_API UVoxelWorldGenerator : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class VOXEL_API IVoxelWorldGenerator
{
	GENERATED_IINTERFACE_BODY()

public:
	/**
	 * Get default value at position
	 * @param	Position		Position in voxel space
	 * @return	Value at position: positive if empty, negative if filled
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);

	/**
	 * Get default color at position
	 * @param	Position		Position in voxel space
	 * @return	Color at position
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);

	/**
	 * If need reference to Voxel World
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);
};
