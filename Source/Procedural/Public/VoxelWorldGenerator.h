// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VoxelWorldGenerator.generated.h"

/**
 *
 */
UCLASS(Blueprintable)
class PROCEDURAL_API UVoxelWorldGenerator : public UObject
{
	GENERATED_BODY()

public:
	// -127 <= value <= 127; value < 0: full; value > 0: empty
	virtual int GetDefaultValue(FIntVector Position);
	virtual FColor GetDefaultColor(FIntVector Position);
};
