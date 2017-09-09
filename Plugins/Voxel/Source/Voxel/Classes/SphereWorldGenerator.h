// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "SphereWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API ASphereWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	ASphereWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FColor GetDefaultColor(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	// Radius of the sphere in world space
	UPROPERTY(EditAnywhere)
		float Radius;

	UPROPERTY(EditAnywhere)
		FColor DefaultColor;

	// If true, sphere is a hole in a full world
	UPROPERTY(EditAnywhere)
		bool InverseOutsideInside;

	UPROPERTY(EditAnywhere)
		float ValueMultiplier;

private:
	float LocalRadius;
};
