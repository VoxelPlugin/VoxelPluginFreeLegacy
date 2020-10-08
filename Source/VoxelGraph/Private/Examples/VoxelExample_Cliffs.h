// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_Cliffs.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Cliffs : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(DisplayName="Cliffs Slope"))
	float Cliffs_Slope = 10.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(DisplayName="Height"))
	float Height = 50.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(DisplayName="Overhangs", UIMax="1", UIMin="0"))
	float Overhangs = 0.2;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base Shape", meta=(DisplayName="Base Shape Frequency", UIMax="1", UIMin="0"))
	float Base_Shape_Frequency = 0.005;
	// < 0 : more holes; > 0: less holes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Base Shape", meta=(DisplayName="Base Shape Offset", UIMax="10", UIMin="-10"))
	float Base_Shape_Offset = 0.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Base Shape Seed"))
	int32 Base_Shape_Seed = 3323;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Sides Noise Seed"))
	int32 Sides_Noise_Seed = 2647;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Top Noise Seed"))
	int32 Top_Noise_Seed = 12932;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Side Noise", meta=(DisplayName="Sides Noise Amplitude", UIMax="1", UIMin="0"))
	float Sides_Noise_Amplitude = 0.2;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Side Noise", meta=(DisplayName="Sides Noise Frequency", UIMax="1", UIMin="0"))
	float Sides_Noise_Frequency = 0.1;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise", meta=(DisplayName="Top Noise Frequency", UIMax="1", UIMin="0"))
	float Top_Noise_Frequency = 0.01;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise", meta=(DisplayName="Top Noise Scale"))
	float Top_Noise_Scale = 25.0;
	
	UVoxelExample_Cliffs();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
