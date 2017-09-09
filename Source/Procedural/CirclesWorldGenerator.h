// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldGenerator.h"
#include "CirclesWorldGenerator.generated.h"

UCLASS(Blueprintable)
class PROCEDURAL_API ACirclesWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	ACirclesWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FColor GetDefaultColor(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	UPROPERTY(EditAnywhere)
		FVector2D Scale;

	UPROPERTY(EditAnywhere)
		FColor TopColor;

	UPROPERTY(EditAnywhere)
		FColor MiddleColor;

	UPROPERTY(EditAnywhere)
		FColor BottomColor;

	UPROPERTY(EditAnywhere)
		float MaxValue;
	UPROPERTY(EditAnywhere)
		float MinValue;
};
