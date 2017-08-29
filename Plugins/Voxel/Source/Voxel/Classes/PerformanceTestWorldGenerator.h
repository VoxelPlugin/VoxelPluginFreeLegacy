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
class VOXEL_API UPerformanceTestWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);
	virtual float GetDefaultValue_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override
	{
	};

	// Radius of the spheres
	UPROPERTY(EditAnywhere)
		int Radius = 5;

	// Speed of the change
	UPROPERTY(EditAnywhere)
		float Speed = 0.01f;
};
