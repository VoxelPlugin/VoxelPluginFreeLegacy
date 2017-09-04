// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "EmptyWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UEmptyWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);



	virtual float GetDefaultValue_Implementation(FIntVector Position) override
	{
		return 0;
	}

	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override
	{
		return FColor::Black;
	}

	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override
	{
	};
};
