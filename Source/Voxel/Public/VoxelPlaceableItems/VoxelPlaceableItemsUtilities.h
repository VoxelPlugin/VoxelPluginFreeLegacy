// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPlaceableItemsUtilities.generated.h"

class AVoxelWorld;
class UVoxelGenerator;

USTRUCT(BlueprintType)
struct FVoxelPerlinWormsSettings
{
	GENERATED_BODY()
	
	// The random seed to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	int32 Seed = 2727;
	
	// The radius of the worms in voxel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	float Radius = 25;

	// Start of the worms, in voxel space
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	FVector Start = FVector(0, 0, 0);
	
	// Direction of the first worm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	FVector Direction = FVector(1, 1, -1);
	
	// The amplitude of the random rotation on each worm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	FVector RotationAmplitude = FVector(10, 10, 90);
	
	// Max depth of the worms tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	int32 NumSegments = 100;
	
	// Length of the worms in voxel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	float SegmentLength = 50;
	
	// Probability of a worm to create 2 worms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (UIMin = 0, UIMax = 1))
	float SplitProbability = 0.1f;
	
	// How much SplitProbability is reduced when worms go deeper in the tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (UIMin = 0, UIMax = 1))
	float SplitProbabilityGain = 0.1f;
	
	// Controls the size of the branches after a split
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm")
	int32 BranchMeanSize = 25;
	
	// Controls the size of the branches after a split, relative to BranchMeanSize
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (UIMin = 0, UIMax = 1))
	float BranchSizeVariation = 0.1f;

public:
	// Perlin noise traversal direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (AdvancedDisplay))
	FVector NoiseDirection = FVector(1, 1, 1);
	
	// Segment lengths to use for perlin noise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (AdvancedDisplay))
	float NoiseSegmentLength = 10;

public:
	// To avoid infinite loops
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perlin Worm", meta = (AdvancedDisplay))
	int32 MaxWorms = 1000;
};

UCLASS()
class VOXEL_API UVoxelPlaceableItemsUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FAddWorm, FVector, Start, FVector, End, float, Radius);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Placeable Items", meta = (DefaultToSelf = World, AdvancedDisplay = "NoiseDir, NoiseSegmentLength"))
	static void AddWorms(FAddWorm AddWorm, FVoxelPerlinWormsSettings Settings);
};