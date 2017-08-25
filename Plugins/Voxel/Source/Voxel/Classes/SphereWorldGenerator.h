// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "SphereWorldGenerator.generated.h"

/**
 * 
 */
UCLASS()
class VOXEL_API USphereWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()
	
public:
	float GetDefaultValue(FIntVector Position) override;
	FColor GetDefaultColor(FIntVector Position) override;

	// Radius of the sphere
	UPROPERTY(EditAnywhere)
		float Radius;

	// If true, sphere is a hole in a full world
	UPROPERTY(EditAnywhere)
		bool InverseOutsideInside;
};
