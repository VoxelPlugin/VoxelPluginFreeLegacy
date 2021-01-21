// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFoliageInterface.h"
#include "VoxelFoliageCollection.generated.h"

class UVoxelFoliage;

UCLASS(BlueprintType)
class VOXELFOLIAGE_API UVoxelFoliageCollection : public UVoxelFoliageCollectionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<UVoxelFoliage*> Foliages;
};