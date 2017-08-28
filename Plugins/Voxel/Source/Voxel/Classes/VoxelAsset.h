#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "VoxelAsset.generated.h"

UCLASS(BlueprintType, HideCategories = (Object))
class VOXEL_API UVoxelAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		FVoxelWorldSave Save;
};
