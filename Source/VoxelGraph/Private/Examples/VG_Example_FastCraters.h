// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VG_Example_FastCraters.generated.h"

UCLASS(Blueprintable)
class UVG_Example_FastCraters : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	
	UVG_Example_FastCraters();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
