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
	UPerlinNoiseWorldGenerator();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);


	virtual float GetDefaultValue_Implementation(FIntVector Position) override;
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override;

private:
	PerlinNoise Noise;
};
