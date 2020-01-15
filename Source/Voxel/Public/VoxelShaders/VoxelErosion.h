// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "RHIResources.h"
#include "VoxelTexture.h"
#include "VoxelErosion.generated.h"

class FVoxelErosionParameters;
class UTexture2D;

UCLASS(Blueprintable, BlueprintType)
class VOXEL_API UVoxelErosion : public UObject
{
	GENERATED_BODY()

public:
	// Must be a multiple of 32
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    int32 Size = 1024;

	// Time elapsed between each simulation step. Smaller = more stable, but slower
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float DeltaTime = 0.005;

	// The scale of the simulation. Should leave to default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Scale = 1;

	// Gravity, use to compute the speed of the water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Gravity = 10;

	// How much sediment a volume of water can carry
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentCapacity = 0.05;

	// How much sediment is removed from height by the water in each step
	// This controls the "strength" of the erosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentDissolving = 0.001;

	// How much sediment can go from the water to the height
	// This controls how far the sediments are carried
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float SedimentDeposition = 0.0001;

	// Amount of water added per step
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float RainStrength = 2;

	// Controls the evaporation of the water
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion Parameters")
    float Evaporation = 1;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init Parameters")
	FVoxelFloatTexture RainMapInit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init Parameters")
	FVoxelFloatTexture HeightmapInit;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	void Step(int32 Count = 10);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetTerrainHeightTexture();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetWaterHeightTexture();

	UFUNCTION(BlueprintCallable, Category = "Voxel|Erosion")
	FVoxelFloatTexture GetSedimentTexture();

};