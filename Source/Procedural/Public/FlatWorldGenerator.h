// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FlatWorldGenerator.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURAL_API UFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	int GetDefaultValue(FIntVector Position);
	FColor GetDefaultColor(FIntVector Position);

	UPROPERTY(EditAnywhere)
		int Height;
};
