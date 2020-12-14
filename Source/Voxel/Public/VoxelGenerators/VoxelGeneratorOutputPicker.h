// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGeneratorOutputPicker.generated.h"

USTRUCT(BlueprintType)
struct FVoxelGeneratorOutputPicker
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FName Name;

	FVoxelGeneratorOutputPicker() = default;
	FVoxelGeneratorOutputPicker(FName Name)
		: Name(Name)
	{
	}
	FVoxelGeneratorOutputPicker(const ANSICHAR* Name)
		: Name(Name)
	{
	}

	operator FName() const
	{
		return Name;
	}
	bool IsNone() const
	{
		return Name.IsNone();
	}
};