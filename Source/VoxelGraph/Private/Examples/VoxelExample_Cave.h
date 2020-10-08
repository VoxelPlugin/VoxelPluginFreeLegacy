// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_Cave.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Cave : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom Noise", meta=(DisplayName="Bottom Noise Frequency"))
	float Bottom_Noise_Frequency = 0.008;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom Noise", meta=(DisplayName="Bottom Noise Scale"))
	float Bottom_Noise_Scale = 150.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Bottom Noise Seed"))
	int32 Bottom_Noise_Seed = 3024;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Global Height Seed"))
	int32 Global_Height_Seed = 1447;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Top Noise Seed"))
	int32 Top_Noise_Seed = 3022;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise", meta=(DisplayName="Top Noise Frequency"))
	float Top_Noise_Frequency = 0.005;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Top Noise", meta=(DisplayName="Top Noise Scale"))
	float Top_Noise_Scale = 150.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bottom & Top Merge", meta=(DisplayName="Bottom Top Merge Smoothness"))
	float Bottom_Top_Merge_Smoothness = 25.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height", meta=(DisplayName="Global Height Merge Smoothness"))
	float Global_Height_Merge_Smoothness = 15.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height", meta=(DisplayName="Global Height Noise Frequency"))
	float Global_Height_Noise_Frequency = 0.005;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height", meta=(DisplayName="Global Height Noise Scale"))
	float Global_Height_Noise_Scale = 200.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Global Height", meta=(DisplayName="Global Height Offset"))
	float Global_Height_Offset = 150.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings", meta=(DisplayName="Cave Height"))
	float Cave_Height = 100.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings", meta=(DisplayName="Cave Radius"))
	float Cave_Radius = 400.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cave Settings", meta=(DisplayName="Cave Walls Smoothness"))
	float Cave_Walls_Smoothness = 100.0;
	
	UVoxelExample_Cave();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
