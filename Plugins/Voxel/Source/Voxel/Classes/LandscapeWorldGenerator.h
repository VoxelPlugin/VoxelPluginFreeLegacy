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
class VOXEL_API ALandscapeWorldGenerator : public AVoxelWorldGenerator
{
	GENERATED_BODY()

public:
	ALandscapeWorldGenerator();

	virtual float GetDefaultValue(int X, int Y, int Z) override;
	virtual FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z) override;
	virtual void SetVoxelWorld(AVoxelWorld* VoxelWorld) override;


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