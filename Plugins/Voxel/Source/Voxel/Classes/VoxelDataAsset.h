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

	UPROPERTY(VisibleAnywhere)
		uint32 SizeX;
	UPROPERTY(VisibleAnywhere)
		uint32 SizeY;
	UPROPERTY(VisibleAnywhere)
		uint32 SizeZ;

	UPROPERTY()
		TArray<float> Values;
	UPROPERTY()
		TArray<FVoxelMaterial> Materials;

	UPROPERTY()
		TArray<bool> DirtyVoxels;
};
