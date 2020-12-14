// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFoliageCollection.generated.h"

class UVoxelFoliage;

UCLASS(BlueprintType)
class VOXEL_API UVoxelFoliageCollection : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", Instanced)
	TArray<UVoxelFoliage*> Foliages;
};