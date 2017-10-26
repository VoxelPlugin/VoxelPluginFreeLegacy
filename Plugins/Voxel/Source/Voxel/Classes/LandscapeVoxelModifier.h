// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerator.h"
#include "Landscape.h"
#include "LandscapeLayerInfoObject.h"
#include "VoxelModifier.h"
#include "LandscapeVoxelModifier.generated.h"

class UVoxelLandscapeAsset;

/**
*
*/
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "Hide"))
class VOXEL_API ALandscapeVoxelModifier : public AVoxelModifier
{
	GENERATED_BODY()

public:
	ALandscapeVoxelModifier();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		ALandscape* Landscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		TArray<ULandscapeLayerInfoObject*> LayerInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import")
		UVoxelLandscapeAsset* ExportedLandscape;

	// Needed because of MinimalAPI on UVoxelLandscapeAsset
	void InitExportedLandscape(TArray<float>& Heights, TArray<FVoxelMaterial>& Materials, int Size);
};