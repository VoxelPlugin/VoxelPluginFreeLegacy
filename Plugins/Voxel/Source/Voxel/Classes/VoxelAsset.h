#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "LandscapeLayerInfoObject.h"
#include "VoxelAsset.generated.h"

class ALandscape;

UCLASS(BlueprintType)
class VOXEL_API AVoxelAsset : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ULandscapeLayerInfoObject* LayerInfo1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ULandscapeLayerInfoObject* LayerInfo2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ULandscapeLayerInfoObject* LayerInfo3;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		FVoxelWorldSave Save;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		bool bIsDirty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		int Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<float> Heights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hide")
		TArray<FColor> Weights;
};
