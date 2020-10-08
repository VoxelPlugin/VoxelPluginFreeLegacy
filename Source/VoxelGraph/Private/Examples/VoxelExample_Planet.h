// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_Planet.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Planet : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Frequency"))
	float Frequency = 2.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Seed"))
	int32 Noise_Seed = 1443;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Strength"))
	float Noise_Strength = 0.02;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="PlanetColorCurve"))
	TSoftObjectPtr<UCurveLinearColor> PlanetColorCurve = TSoftObjectPtr<UCurveLinearColor>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/Planet/VoxelExample_Planet_ColorCurve.VoxelExample_Planet_ColorCurve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="PlanetCurve"))
	TSoftObjectPtr<UCurveFloat> PlanetCurve = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/Planet/VoxelExample_Planet_Curve.VoxelExample_Planet_Curve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Radius"))
	float Radius = 1000.0;
	
	UVoxelExample_Planet();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
