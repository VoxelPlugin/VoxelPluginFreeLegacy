// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphGeneratorHelpers.h"
#include "VoxelNodeFunctions.h"
#include "FastNoise.h"
#include "Containers/StaticArray.h"
#include "VoxelExample_Cave.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Cave : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom Noise")
	float Bottom_Noise_Frequency = 0.008000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom Noise")
	float Bottom_Noise_Scale = 150.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise")
	float Top_Noise_Frequency = 0.005000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise")
	float Top_Noise_Scale = 150.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom & Top Merge")
	float Bottom_Top_Merge_Smoothness = 25.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height")
	float Global_Height_Offset = 150.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height")
	float Global_Height_Merge_Smoothness = 15.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height")
	float Global_Height_Noise_Frequency = 0.005000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height")
	float Global_Height_Noise_Scale = 200.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings")
	float Cave_Height = 100.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings")
	float Cave_Walls_Smoothness = 100.000000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings")
	float Cave_Radius = 400.000000;
	
	UVoxelExample_Cave();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
