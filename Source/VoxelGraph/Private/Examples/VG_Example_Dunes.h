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
	// The direction of the noise. Will be normalized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Direction X"))
	float Direction_X = 0.4;
	// The direction of the noise. Will be normalized
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Direction Y"))
	float Direction_Y = 1.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Dune Frequency"))
	float Dune_Frequency = 0.002;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Height"))
	float Height = 75.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Frequency"))
	float Noise_Frequency = 0.001;
	
	UVG_Example_Dunes();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
