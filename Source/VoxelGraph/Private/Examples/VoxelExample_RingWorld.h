// Copyright 2021 Phyronnaz

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
	FLinearColor BeachColor = FLinearColor(0.989583, 0.833626, 0.282215, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorHigh"))
	FLinearColor MountainsColorHigh = FLinearColor(1.000000, 1.000000, 1.000000, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorLowHigh"))
	FLinearColor MountainsColorLowHigh = FLinearColor(0.129469, 0.136312, 0.140625, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="MountainsColorLowLow"))
	FLinearColor MountainsColorLowLow = FLinearColor(0.036458, 0.023406, 0.017120, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Curves", meta=(DisplayName="MoutainsMaskCurve"))
	TSoftObjectPtr<UCurveFloat> MoutainsMaskCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/RingWorld/MoutainsMaskCurve.MoutainsMaskCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="PlainsColorHigh"))
	FLinearColor PlainsColorHigh = FLinearColor(0.104616, 0.184475, 0.042311, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="PlainsColorLow"))
	FLinearColor PlainsColorLow = FLinearColor(0.041351, 0.072917, 0.016724, 1.000000);
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
	FLinearColor RingOuterColor = FLinearColor(0.005208, 0.000000, 0.000099, 1.000000);
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Colors", meta=(DisplayName="RiverColor"))
	FLinearColor RiverColor = FLinearColor(0.000000, 0.000000, 1.000000, 1.000000);
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
