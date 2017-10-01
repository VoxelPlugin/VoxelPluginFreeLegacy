// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "FunnyWorldGenerator.generated.h"

UCLASS(Blueprintable)
class PROCEDURAL_API AFunnyWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	AFunnyWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial TopMaterial;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial MiddleMaterial;

	UPROPERTY(EditAnywhere)
		FVoxelMaterial BottomMaterial;

	UPROPERTY(EditAnywhere)
		float ValueMultiplier;

	UPROPERTY(EditAnywhere)
		int SphereLayerHeight;

	UPROPERTY(EditAnywhere)
		int Radius;

	UPROPERTY(EditAnywhere)
		float RadiusDivisor;
};
