// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_LayeredPlanet.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_LayeredPlanet : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	float Frequency = 0.005000;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	TSoftObjectPtr<UCurveFloat> None1 = TSoftObjectPtr<UCurveFloat>(FSoftObjectPath("/Voxel/Examples/VoxelGraphs/PerlinWorms/VoxelExample_LayeredWorld_Curve.VoxelExample_LayeredWorld_Curve"));
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="No Category")
	bool Slice_Mode = false;
	
	UVoxelExample_LayeredPlanet();
	virtual TMap<FName, int32> GetDefaultSeeds() const override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
};
