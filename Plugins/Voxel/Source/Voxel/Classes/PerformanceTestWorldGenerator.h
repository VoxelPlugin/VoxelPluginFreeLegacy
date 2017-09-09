// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "RenderCore.h"
#include "PerformanceTestWorldGenerator.generated.h"


/**
*
*/
UCLASS(Blueprintable)
class VOXEL_API APerformanceTestWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FColor GetDefaultColor(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;

	// Radius of the spheres
	UPROPERTY(EditAnywhere)
		int Radius = 5;

	// Speed of the change
	UPROPERTY(EditAnywhere)
		float Speed = 0.01f;

private:
	AVoxelWorld* World;
};
