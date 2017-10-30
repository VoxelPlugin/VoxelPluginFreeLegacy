// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"
#include "LandscapeLayerInfoObject.h"
#include "VoxelLandscapeImporter.generated.h"

class UVoxelLandscapeAsset;

/**
 * Actor that create a UVoxelLandscapeAsset from an ALandscape
 */
UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering"))
class VOXEL_API AVoxelLandscapeImporter : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Save Path: /Game/", RelativeToGameContentDir), Category = "Save configuration")
		FDirectoryPath SavePath;

	UPROPERTY(EditAnywhere, Category = "Save configuration")
		FString FileName;


	UPROPERTY(EditAnywhere, Category = "Import configuration")
		ALandscape* Landscape;

	// TODO: Custom materials
	UPROPERTY(EditAnywhere, Category = "Import configuration")
		TArray<ULandscapeLayerInfoObject*> LayerInfos;
};