// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPlaceableItemActor.generated.h"

class AVoxelWorld;

UCLASS(Abstract, HideCategories = ("Tick", "Replication", "Input", "Actor", "Rendering", "HOLD", "LOD", "Cooking", "Collision"))
class VOXEL_API AVoxelPlaceableItemActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Placeable Item Actor Settings")
	AVoxelWorld* PreviewWorld;

	// If true, will only affect PreviewWorld. If false, will affect all the voxel worlds spawned into the scene
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Placeable Item Actor Settings")
	bool bOnlyImportIntoPreviewWorld = true;

};