// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelModifier.h"
#include "LandscapeVoxelModifier.generated.h"

/**
*
*/
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API ALandscapeVoxelModifier : public AVoxelModifier
{
	GENERATED_BODY()

public:
	ALandscapeVoxelModifier();
};