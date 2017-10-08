// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelModifier.h"
#include "VoxelAssets/LandscapeVoxelAsset.h"
#include "LandscapeVoxelModifier.generated.h"

class AVoxelWorld;
class FVoxelData;
class AVoxelPart;

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

	UPROPERTY(EditAnywhere)
		AVoxelWorld* PreviewWorld;

	UPROPERTY(EditAnywhere)
		bool bEnablePreview;

	virtual void ApplyToWorld(AVoxelWorld* World) override;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
		AVoxelPart* Part;

	void UpdateRender();
};
