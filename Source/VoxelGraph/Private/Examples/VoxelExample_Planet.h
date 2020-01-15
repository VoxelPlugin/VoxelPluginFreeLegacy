// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "VoxelExample_Planet.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Planet : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Frequency = 2.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Radius = 1000.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UCurveFloat> PlanetCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/Planet/VoxelExample_Planet_Curve.VoxelExample_Planet_Curve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UCurveLinearColor> PlanetColorCurve = TSoftObjectPtr<UCurveLinearColor>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/Planet/VoxelExample_Planet_ColorCurve.VoxelExample_Planet_ColorCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Noise_Strength = 0.020000;
	
	UVoxelExample_Planet();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
