// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_LayeredWorld.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_LayeredWorld : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Frequency"))
	float Frequency = 0.005;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName))
	TSoftObjectPtr<UCurveFloat> None1 = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/PerlinWorms/VoxelExample_LayeredWorld_Curve.VoxelExample_LayeredWorld_Curve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Seed"))
	int32 Seed = 4761;
	
	UVoxelExample_LayeredWorld();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
