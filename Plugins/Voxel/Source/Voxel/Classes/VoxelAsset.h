#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "VoxelAsset.generated.h"

class ALandscape;

UCLASS(BlueprintType)
class VOXEL_API AVoxelAsset : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		ALandscape* Landscape;


	UPROPERTY()
		FVoxelWorldSave Save;

	UPROPERTY(BlueprintReadonly)
		bool bIsDirty;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
		int Width;

	UPROPERTY(BlueprintReadOnly)
		TArray<float> Heights;

	void CreateFromLandcape();
};
