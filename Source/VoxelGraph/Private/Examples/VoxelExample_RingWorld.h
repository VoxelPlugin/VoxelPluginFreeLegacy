// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_RingWorld.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_RingWorld : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Radius"))
	float Radius = 7000.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="RingEdgesHardness"))
	float RingEdgesHardness = 10.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Scale"))
	float Scale = 10.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Thickness"))
	float Thickness = 500.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Width in Degrees"))
	float Width_in_Degrees = 50.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="RiverDepth"))
	float RiverDepth = 100.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="RiverWidth"))
	float RiverWidth = 1.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="BeachColor"))
	FColor BeachColor = FColor(253, 213, 72, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorHigh"))
	FColor MountainsColorHigh = FColor(255, 255, 255, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorLowHigh"))
	FColor MountainsColorLowHigh = FColor(33, 34, 35, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorLowLow"))
	FColor MountainsColorLowLow = FColor(9, 5, 4, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves", meta=(DisplayName="MoutainsMaskCurve"))
	TSoftObjectPtr<UCurveFloat> MoutainsMaskCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/MoutainsMaskCurve.MoutainsMaskCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="PlainsColorHigh"))
	FColor PlainsColorHigh = FColor(26, 47, 10, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="PlainsColorLow"))
	FColor PlainsColorLow = FColor(10, 18, 4, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="PlainsNoiseFrequency"))
	float PlainsNoiseFrequency = 0.2;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="PlainsNoiseHeight"))
	float PlainsNoiseHeight = 250.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves", meta=(DisplayName="PlainsNoiseStrengthCurve"))
	TSoftObjectPtr<UCurveFloat> PlainsNoiseStrengthCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/PlainsNoiseStrengthCurve.PlainsNoiseStrengthCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves", meta=(DisplayName="RingMainShapeCurve"))
	TSoftObjectPtr<UCurveFloat> RingMainShapeCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/RingMainShapeCurve.RingMainShapeCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="RingOuterColor"))
	FColor RingOuterColor = FColor(1, 0, 0, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="RiverColor"))
	FColor RiverColor = FColor(0, 0, 255, 255);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves", meta=(DisplayName="RiverDepthCurve"))
	TSoftObjectPtr<UCurveFloat> RiverDepthCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/RiverDepthCurve.RiverDepthCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="MountainsNoiseFrequency"))
	float MountainsNoiseFrequency = 0.2;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="MountainsNoiseHeight"))
	float MountainsNoiseHeight = 500.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="BaseNoiseFrquency"))
	float BaseNoiseFrquency = 0.005;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="BaseNoiseHeight"))
	float BaseNoiseHeight = 250.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="BaseHeight"))
	float BaseHeight = 1000.0;
	
	UVoxelExample_RingWorld();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
