// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VG_Example_Dunes.generated.h"

UCLASS(Blueprintable)
class UVG_Example_Dunes : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Dune_Frequency = 0.002000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Noise_Frequency = 0.001000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Height = 75.000000;
	// The direction of the noise. Will be normalized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Direction_X = 0.400000;
	// The direction of the noise. Will be normalized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Direction_Y = 1.000000;
	
	UVG_Example_Dunes();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
