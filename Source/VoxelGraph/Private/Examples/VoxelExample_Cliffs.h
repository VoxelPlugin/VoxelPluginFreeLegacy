// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "VoxelExample_Cliffs.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Cliffs : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float Cliffs_Slope = 10.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	float Height = 50.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(UIMax="1", UIMin="0"))
	float Overhangs = 0.200000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base Shape", meta=(UIMax="1", UIMin="0"))
	float Base_Shape_Frequency = 0.005000;
	// < 0 : more holes; > 0: less holes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base Shape", meta=(UIMax="10", UIMin="-10"))
	float Base_Shape_Offset = 0.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Side Noise", meta=(UIMax="1", UIMin="0"))
	float Sides_Noise_Frequency = 0.100000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Side Noise", meta=(UIMax="1", UIMin="0"))
	float Sides_Noise_Amplitude = 0.200000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise")
	float Top_Noise_Scale = 25.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise", meta=(UIMax="1", UIMin="0"))
	float Top_Noise_Frequency = 0.010000;
	
	UVoxelExample_Cliffs();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
