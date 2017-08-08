// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "FlatWorldGenerator.generated.h"


USTRUCT(Blueprintable)
struct VOXEL_API FFlatWorldStruct
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		FLinearColor Color;

	UPROPERTY(EditAnywhere)
		int Start;

	UPROPERTY(EditAnywhere)
		int Height;
};

/**
 *
 */
UCLASS()
class VOXEL_API UFlatWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	int GetDefaultValue(FIntVector Position);
	FColor GetDefaultColor(FIntVector Position);

	UPROPERTY(EditAnywhere)
		int Height;

	UPROPERTY(EditAnywhere)
		FLinearColor DefaultColor;

	UPROPERTY(EditAnywhere)
		TArray<FFlatWorldStruct> Layers;

};