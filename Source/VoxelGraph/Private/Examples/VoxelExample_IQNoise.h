// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "VoxelExample_IQNoise.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_IQNoise : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Height = 500.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Frequency = 0.001000;
	
	UVoxelExample_IQNoise();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
