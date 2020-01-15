// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPlaceableItemsUtilities.generated.h"

class AVoxelWorld;

UCLASS()
class UVoxelPlaceableItemsUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Add perlin worms items to the voxel world. You can use them using the Perlin Worms Distance node
	 * @param World					The voxel world
	 * @param Radius				The radius of the worms in voxel
	 * @param Seed					The random seed to use
	 * @param RotationAmplitude		The amplitude of the random rotation on each worm
	 * @param NoiseDir				Perlin noise traversal direction
	 * @param NoiseSegmentLength	Segment lengths to use for perlin noise
	 * @param Start					Start of the worms, in voxel space
	 * @param InitialDir			Initial direction of the worms
	 * @param VoxelSegmentLength	Length of the worms in voxel
	 * @param NumSegments			Max depth of the worms tree
	 * @param SplitProbability		Probability of a worm to create 2 worms
	 * @param SplitProbabilityGain	How much SplitProbability is reduced when worms go deeper in the tree
	 * @param BranchMeanSize		Controls the size of the branches after a split
	 * @param BranchSizeVariation	Controls the size of the branches after a split
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Placeable Items", meta = (DefaultToSelf = World, AdvancedDisplay = "NoiseDir, NoiseSegmentLength"))
	static void AddWorms(
		AVoxelWorld* World,
		float Radius = 25,
		int32 Seed = 2727, 
		FVector RotationAmplitude = FVector(10, 10, 90), 
		FVector NoiseDir = FVector(1, 1, 1), 
		float NoiseSegmentLength = 10, 
		FVector Start = FVector(0, 0,0), 
		FVector InitialDir = FVector(1, 1, -1), 
		float VoxelSegmentLength = 50, 
		int32 NumSegments = 100, 
		float SplitProbability = 0.1, 
		float SplitProbabilityGain = 0.1, 
		int32 BranchMeanSize = 25, 
		int32 BranchSizeVariation = 10);
};