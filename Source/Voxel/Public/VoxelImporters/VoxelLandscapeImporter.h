// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "GameFramework/Actor.h"
#include "VoxelLandscapeImporter.generated.h"

class ULandscapeLayerInfoObject;
class ALandscape;

USTRUCT()
struct VOXEL_API FVoxelLandscapeImporterLayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	ULandscapeLayerInfoObject* LayerInfo = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelRGBA Layer = EVoxelRGBA::R;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	uint8 Index = 0;
};

UCLASS(BlueprintType, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HOLD", "LOD", "Cooking"))
class VOXEL_API AVoxelLandscapeImporter : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Import configuration")
	ALandscape* Landscape = nullptr;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	EVoxelHeightmapImporterMaterialConfig MaterialConfig = EVoxelHeightmapImporterMaterialConfig::RGB;

	UPROPERTY(EditAnywhere, Category = "Import configuration")
	TArray<FVoxelLandscapeImporterLayerInfo> LayerInfos;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};