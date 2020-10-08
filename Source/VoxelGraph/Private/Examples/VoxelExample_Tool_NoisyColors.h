// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VoxelExample_Tool_NoisyColors.generated.h"

UCLASS(Blueprintable)
class UVoxelExample_Tool_NoisyColors : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="", meta=(DisplayName="Color"))
	FLinearColor Color = FLinearColor(0.056128, 0.109462, 0.052861, 1.000000);
	
	UVoxelExample_Tool_NoisyColors();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
