// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "Landscape.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeVoxelAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API ALandscapeVoxelAsset : public AActor
{
	GENERATED_BODY()

public:
	ALandscapeVoxelAsset();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		TArray<ULandscapeLayerInfoObject*> LayerInfos;


	// Higher precision can improve render quality, but voxel values are lower (mining speed not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float ValueMultiplier;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<float> Heights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<FColor> Weights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		int Size;


	void Init(float InVoxelSize);

	float GetDefaultValue(int X, int Y, int Z);
	FVoxelMaterial GetDefaultMaterial(int X, int Y, int Z);

private:
	float VoxelSize;
};