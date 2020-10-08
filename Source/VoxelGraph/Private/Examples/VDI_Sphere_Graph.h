// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratedWorldGeneratorsIncludes.h"
#include "VDI_Sphere_Graph.generated.h"

UCLASS(Blueprintable)
class UVDI_Sphere_Graph : public UVoxelGraphGeneratorHelper
{
	GENERATED_BODY()
	
public:
	
	UVDI_Sphere_Graph();
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
};
