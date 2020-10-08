// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_HollowPlanet.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_HollowPlanet : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Intersection Smoothness"))
	float Intersection_Smoothness = 10.0;
	// Above 0: More ground. Below zero: less ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Bias"))
	float Noise_Bias = 0.2;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Frequency"))
	float Noise_Frequency = 4.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Scale"))
	float Noise_Scale = 20.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Radius"))
	float Radius = 250.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Seed"))
	int32 Seed = 1443;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Use IQ Noise"))
	bool Use_IQ_Noise = true;
	
	UVoxelExample_HollowPlanet();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
