// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IQueuedWork.h"
#include "VoxelCraterTools.generated.h"

class AVoxelWorld;
class FVoxelData;

/**
 * Async task to add a crater
 */
class FAsyncAddCrater : public IQueuedWork
{
public:
	FAsyncAddCrater(FVoxelData* Data, const FIntVector& LocalPosition, int IntRadius, float Radius, uint8 BlackMaterialIndex, uint8 AddedBlack, float HardnessMultiplier, AVoxelWorld* World);

	//~ Begin IQueuedWork Interface
	void DoThreadedWork() override;
	void Abandon() override;
	//~ End IQueuedWork Interface

private:
	FVoxelData* const Data;
	const FIntVector LocalPosition;
	const int IntRadius;
	const float Radius;
	const uint8 BlackMaterialIndex;
	const uint8 AddedBlack;
	const float HardnessMultiplier;
	AVoxelWorld* const World;
};

/**
 * Crater blueprint tools for voxels
 */
UCLASS()
class VOXEL_API UVoxelCraterTools : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Add a crater
	 * @param	World				Voxel World
	 * @param	Position			Center of the crater
	 * @param	Radius				Radius of the crater
	 * @param	BlackMaterialIndex	The material index to set inside the crater
	 * @param	AddedBlack			Amount of black to add (@see BlackMaterialIndex)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "5"))
	static void AddCrater(AVoxelWorld* World, FVector Position, float Radius, uint8 BlackMaterialIndex, uint8 AddedBlack = 200);
	/**
	 * Add a crater in an async thread
	 * @param	World				Voxel World
	 * @param	Position			Center of the crater
	 * @param	Radius				Radius of the crater
	 * @param	BlackMaterialIndex	The material index to set inside the crater
	 * @param	AddedBlack			Amount of black to add (@see BlackMaterialIndex)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void AddCraterMultithreaded(AVoxelWorld* World, FVector Position, float Radius, uint8 BlackMaterialIndex, uint8 AddedBlack = 200);
};