// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "RenderCore.h"
#include "PerformanceTestWorldGenerator.generated.h"


/**
*
*/
UCLASS()
class VOXEL_API UPerformanceTestWorldGenerator : public UVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	float GetDefaultValue(FIntVector Position) override
	{
		float CurrentRadius = (1 + FMath::Sin(Speed * (FDateTime::Now().GetSecond() + FDateTime::Now().GetMillisecond() / 1000.f))) * Radius / 2;
		return FVector(FMath::Abs(Position.X) % (2 * Radius) - Radius, FMath::Abs(Position.Y) % (2 * Radius) - Radius, FMath::Abs(Position.Z) % (2 * Radius) - Radius).Size() - CurrentRadius;
	}

	FColor GetDefaultColor(FIntVector Position) override
	{
		return FColor::Green;
	}

	// Radius of the spheres
	UPROPERTY(EditAnywhere)
		int Radius = 5;

	// Speed of the change
	UPROPERTY(EditAnywhere)
		float Speed = 0.01f;
};