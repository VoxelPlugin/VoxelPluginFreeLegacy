// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "VoxelAsset.h"
#include "TerrainImporterWorldGenerator.generated.h"


UENUM(BlueprintType)
enum class EBlur : uint8
{
	BE_NoBlur			UMETA(DisplayName = "No Blur"),
	BE_GaussianBlur 	UMETA(DisplayName = "Gaussian Blur"),
	BE_NormalBLur		UMETA(DisplayName = "Normal Blur")
};


/**
 *
 */
UCLASS(Blueprintable)
class VOXEL_API UTerrainImporterWorldGenerator : public UObject, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	UTerrainImporterWorldGenerator() : TerrainObject(nullptr), Center(0, 0), InValue(-1), OutValue(1)
	{
	};

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);
	virtual float GetDefaultValue_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override;

	UPROPERTY(EditAnywhere)
		TSubclassOf<AVoxelAsset> Terrain;

	UPROPERTY(EditAnywhere)
		FVector2D Center;

	UPROPERTY(EditAnywhere)
		float InValue;

	UPROPERTY(EditAnywhere)
		float OutValue;

	UPROPERTY(EditAnywhere)
		EBlur Blur;

private:
	AVoxelAsset* TerrainObject;
	AVoxelWorld* World;
};