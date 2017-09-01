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
	ALandscapeWorldGenerator() : Precision(1), MinValue(-1), MaxValue(1)
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


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ULandscapeLayerInfoObject* LayerInfo1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ULandscapeLayerInfoObject* LayerInfo2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ULandscapeLayerInfoObject* LayerInfo3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ULandscapeLayerInfoObject* LayerInfo4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Import")
		FVector LandscapePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Import")
		int Width;


	// Higher precision can improve render quality, but voxel values are lower (mining speed not constant)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"))
		int Precision;

	UPROPERTY(EditAnywhere)
		float MinValue;

	UPROPERTY(EditAnywhere)
		float MaxValue;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<float> Heights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<FColor> Weights;

private:
	AVoxelWorld* World;
	FVector LocalLandscapePosition;
	FVector LocalLandscapePositionAndWidth;
};