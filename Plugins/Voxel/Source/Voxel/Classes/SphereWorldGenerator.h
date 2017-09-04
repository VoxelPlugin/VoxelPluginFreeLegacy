// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "SphereWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API USphereWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	USphereWorldGenerator();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);


	virtual float GetDefaultValue_Implementation(FIntVector Position) override;
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override;

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
