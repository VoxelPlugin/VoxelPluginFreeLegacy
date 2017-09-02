// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "PerlinNoise.h"
#include "PerlinNoiseWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UPerlinNoiseWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:

	UPerlinNoiseWorldGenerator() : Noise()
	{
	};

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);
	virtual float GetDefaultValue_Implementation(FIntVector Position) override
	{
		float n = Noise.Noise(Position.X / 10.f, Position.Y / 10.f, Position.Z / 10.f);
		n = n - FMath::FloorToInt(n);

		return 2 * n - 1;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override
	{
		return FColor::Green;
	}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override
	{
	};

	PerlinNoise Noise;
};
