// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "VoxelExample_Ravines.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Ravines : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Bottom_Transition_Smoothness = 5.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category", meta=(UIMax="1", UIMin="0"))
	float _3D_Noise_Frequency = 0.020000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Height = 50.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Top_Transition_Smoothness = 5.000000;
	
	UVoxelExample_Ravines();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
