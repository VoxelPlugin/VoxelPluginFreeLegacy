// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VDI_Ravine_Graph.generated.h"

UCLASS(Blueprintable)
class UVDI_Ravine_Graph : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	
	UVDI_Ravine_Graph();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
