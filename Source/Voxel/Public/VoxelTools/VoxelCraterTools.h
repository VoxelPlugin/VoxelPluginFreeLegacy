// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "VoxelCraterTools.generated.h"

class AVoxelWorld;

UCLASS()
class VOXEL_API UVoxelCraterTools : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Add a crater
	 * @param	World					Voxel World
	 * @param	Position				Center of the crater
	 * @param	Radius					Radius of the crater in voxels
	 * @param	PositionsSampleRate		Sample random positions inside the crater to eg spawn effects. Probability for a position to be sampled, between 0 and 1
	 * @param	SampledPositions		The sampled positions, see PositionsSampleRate
	 * @param	bUpdateRender			Should the render be updated?
	 * @param   bAllowFailure			If the data is locked by another thread, fail instead of waiting
	 * @return	If the edit was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "PositionsSampleRate, SampledPositions, bUpdateRender, bAllowFailure"))
	static bool AddCrater(
		AVoxelWorld* World, 
		FIntVector Position, 
		float Radius, 
		float PositionsSampleRate, 
		TArray<FIntVector>& SampledPositions,
		bool bUpdateRender = true, 
		bool bAllowFailure = false);
	/**
	 * Add a crater in an async thread
	 * @param	World					Voxel World
	 * @param	Position				Center of the crater
	 * @param	Radius					Radius of the crater in voxels
	 * @param	PositionsSampleRate		Sample random positions inside the crater to eg spawn effects. Probability for a position to be sampled, between 0 and 1
	 * @param	SampledPositions		The sampled positions, see PositionsSampleRate
	 * @param	bUpdateRender			Should the render be updated?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "PositionsSampleRate, SampledPositions, bUpdateRender"))
	static void AddCraterAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		AVoxelWorld* World, 
		const FIntVector& Position, 
		float Radius, 
		float PositionsSampleRate, 
		TArray<FIntVector>& SampledPositions,
		bool bUpdateRender = true);
};