// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "Landscape.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeWorldGenerator.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class VOXEL_API ALandscapeWorldGenerator : public AActor, public IVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	ALandscapeWorldGenerator();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		float GetDefaultValue(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		FColor GetDefaultColor(FIntVector Position);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Voxel")
		void SetVoxelWorld(AVoxelWorld* VoxelWorld);


	virtual float GetDefaultValue_Implementation(FIntVector Position) override;
	virtual FColor GetDefaultColor_Implementation(FIntVector Position) override;
	virtual void SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		TArray<ULandscapeLayerInfoObject*> LayerInfos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Import")
		FVector LandscapePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Import")
		int Width;


	// Higher precision can improve render quality, but voxel values are lower (mining speed not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float ValueMultiplier;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<float> Heights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<FColor> Weights;

private:
	AVoxelWorld* World;
	FVector LocalLandscapePosition;
	FVector LocalLandscapePositionAndWidth;
};