// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_FloatingIslandOnion.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_FloatingIslandOnion : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Top_Noise_Frequency = 0.002000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Perturb_Frequency = 0.010000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Perturb_Amplitude = 50.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Top_Noise_Height = 500.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Height = 200.000000;
	
	UVoxelExample_FloatingIslandOnion();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
