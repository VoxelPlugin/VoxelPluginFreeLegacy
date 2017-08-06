// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "SphereWorldGenerator.generated.h"

/**
 * 
 */
UCLASS()
class PROCEDURAL_API USphereWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()
	
public:
	int GetDefaultValue(FIntVector Position);
	FColor GetDefaultColor(FIntVector Position);

	UPROPERTY(EditAnywhere)
		float Radius;
};
