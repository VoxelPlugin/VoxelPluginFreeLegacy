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
	UPROPERTY(EditAnywhere)
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere)
		ULandscapeLayerInfoObject* LayerInfo1;
	UPROPERTY(EditAnywhere)
		ULandscapeLayerInfoObject* LayerInfo2;
	UPROPERTY(EditAnywhere)
		ULandscapeLayerInfoObject* LayerInfo3;


	UPROPERTY()
		FVoxelWorldSave Save;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
		bool bIsDirty;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
		int Width;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
		TArray<float> Heights;
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
		TArray<FColor> Weights;

	void CreateFromLandcape();
};
