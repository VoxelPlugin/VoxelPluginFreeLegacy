// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelDataAsset.generated.h"


UCLASS(MinimalAPI)
class UVoxelDataAsset : public UObject
{
	GENERATED_BODY()

public:
	UVoxelDataAsset(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere)
		FString Name;

private:
	UPROPERTY()
		TArray<uint8> Data;
};

struct FVoxelDataAsset
{
	uint32 SizeX;
	uint32 SizeY;
	uint32 SizeZ;

	TArray<float> Values;
	TArray<FVoxelMaterial> Materials;

	TArray<bool> DirtyVoxels;
};
