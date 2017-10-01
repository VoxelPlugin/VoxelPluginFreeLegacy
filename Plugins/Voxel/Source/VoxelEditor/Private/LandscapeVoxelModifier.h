// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelModifier.h"
#include "LandscapeVoxelModifier.generated.h"

class AVoxelWorld;
class FVoxelData;
class ALandscapeVoxelAsset;

/**
*
*/
UCLASS(Blueprintable)
class VOXELEDITOR_API ALandscapeVoxelModifier : public AVoxelModifier
{
	GENERATED_BODY()

public:
	ALandscapeVoxelModifier();

	UPROPERTY(EditAnywhere)
		ALandscapeVoxelAsset* Landscape;

	virtual void ApplyToWorld(AVoxelWorld* World) override;
	virtual void Render(FVector WorldPosition, float VoxelSize) override;
};
