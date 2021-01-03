// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphOutputs.h"
#include "VoxelGraphOutputsConfig.generated.h"

UCLASS()
class UVoxelGraphOutputsConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FVoxelGraphOutput> Outputs;
};