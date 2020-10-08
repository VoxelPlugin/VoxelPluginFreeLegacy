// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VDI_Capsule_Graph.generated.h"

UCLASS(Blueprintable)
class UVDI_Capsule_Graph : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// Relative to the radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Noise Amplitude", UIMax="2", UIMin="0"))
	float Noise_Amplitude = 1.0;
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Seed"))
	int32 Seed = 1443;
	
	UVDI_Capsule_Graph();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
