// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_Ravines.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Ravines : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName=" 3D Noise Frequency", UIMax="1", UIMin="0"))
	float _3D_Noise_Frequency = 0.02;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="3D Noise Seed"))
	int32 _3D_Noise_Seed = 1443;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Bottom Transition Smoothness"))
	float Bottom_Transition_Smoothness = 5.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Height"))
	float Height = 50.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Top Transition Smoothness"))
	float Top_Transition_Smoothness = 5.0;
	
	UVoxelExample_Ravines();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
