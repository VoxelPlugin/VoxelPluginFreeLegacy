// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelModifier.generated.h"

class AVoxelWorld;
class FVoxelData;

/**
*
*/
UCLASS(Blueprintable, abstract)
class VOXEL_API AVoxelModifier : public AActor
{
	GENERATED_BODY()

public:
	virtual void ApplyToWorld(AVoxelWorld* World) {};
};
