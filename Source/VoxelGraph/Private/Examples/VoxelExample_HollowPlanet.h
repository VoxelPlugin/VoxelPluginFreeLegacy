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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	bool Use_IQ_Noise = true;
	// Above 0: More ground. Below zero: less ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Noise_Bias = 0.200000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Intersection_Smoothness = 10.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Noise_Frequency = 4.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Noise_Scale = 20.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Radius = 250.000000;
	
	UVoxelExample_HollowPlanet();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
