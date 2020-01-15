// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "Curves/CurveFloat.h"
#include "VoxelExample_RingWorld.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_RingWorld : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Scale = 10.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Radius = 7000.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float RingEdgesHardness = 10.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Width_in_Degrees = 50.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Thickness = 500.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float RiverDepth = 100.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float RiverWidth = 1.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves")
	TSoftObjectPtr<UCurveFloat> RingMainShapeCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/RingMainShapeCurve.RingMainShapeCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves")
	TSoftObjectPtr<UCurveFloat> RiverDepthCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/RiverDepthCurve.RiverDepthCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves")
	TSoftObjectPtr<UCurveFloat> MoutainsMaskCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/MoutainsMaskCurve.MoutainsMaskCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float PlainsNoiseHeight = 250.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor MountainsColorLowLow = FColor(9, 5, 4, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor PlainsColorLow = FColor(10, 18, 4, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves")
	TSoftObjectPtr<UCurveFloat> PlainsNoiseStrengthCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/PlainsNoiseStrengthCurve.PlainsNoiseStrengthCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float PlainsNoiseFrequency = 0.200000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor RiverColor = FColor(0, 0, 255, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor BeachColor = FColor(253, 213, 72, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor MountainsColorLowHigh = FColor(33, 34, 35, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor RingOuterColor = FColor(1, 0, 0, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor PlainsColorHigh = FColor(26, 47, 10, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors")
	FColor MountainsColorHigh = FColor(255, 255, 255, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float MountainsNoiseHeight = 500.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float MountainsNoiseFrequency = 0.200000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float BaseNoiseFrquency = 0.005000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float BaseNoiseHeight = 250.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float BaseHeight = 1000.000000;
	
	UVoxelExample_RingWorld();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
