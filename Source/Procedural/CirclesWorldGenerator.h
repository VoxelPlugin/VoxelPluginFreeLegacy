// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldGenerator.h"
#include "CirclesWorldGenerator.generated.h"

UCLASS(Blueprintable)
class PROCEDURAL_API UCirclesWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UCirclesWorldGenerator();

	//virtual float GetDefaultValue(int X, int Y, int Z) override;
	//virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;
	//virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	UPROPERTY(EditAnywhere)
		FVector2D Scale;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial TopMaterial;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial MiddleMaterial;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial BottomMaterial;

	UPROPERTY(EditAnywhere)
		float MaxValue;
	UPROPERTY(EditAnywhere)
		float MinValue;
};
