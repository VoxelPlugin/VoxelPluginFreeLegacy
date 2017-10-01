// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelModifier.h"
#include "VoxelAssets/LandscapeVoxelAsset.h"
#include "LandscapeVoxelModifier.generated.h"

class AVoxelWorld;
class FVoxelData;

/**
*
*/
UCLASS(Blueprintable, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API ALandscapeVoxelModifier : public AVoxelModifier
{
	GENERATED_BODY()

public:
	ALandscapeVoxelModifier();

	UPROPERTY(EditAnywhere)
		TSubclassOf<ALandscapeVoxelAsset> Landscape;

	virtual void ApplyToWorld(AVoxelWorld* World) override;
	virtual void Render(FVector WorldPosition, float VoxelSize) override;
};
