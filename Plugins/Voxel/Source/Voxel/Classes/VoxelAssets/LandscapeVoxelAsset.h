// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
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

	int GetMinBound(int X, int Y);
	int GetMaxBound(int X, int Y);

	FORCEINLINE float GetDefaultValue(const int X, const int Y, const int Z);
	FORCEINLINE FVoxelMaterial GetDefaultMaterial(const int X, const int Y, const int Z);

private:
	float VoxelSize;
};