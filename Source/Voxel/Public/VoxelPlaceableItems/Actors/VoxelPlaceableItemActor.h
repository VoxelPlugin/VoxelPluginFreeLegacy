// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelPlaceableItemActor.generated.h"

class AVoxelWorld;

UCLASS(Abstract)
class VOXEL_API AVoxelPlaceableItemActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, CallInEditor, Category = "Placeable Item Actor Settings")
	AVoxelWorld* PreviewWorld;

	// If true, will only affect PreviewWorld. If false, will affect all the voxel worlds spawned into the scene
	UPROPERTY(BlueprintReadWrite, EditAnywhere, CallInEditor, Category = "Placeable Item Actor Settings")
	bool bOnlyImportIntoPreviewWorld = true;

public:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "AddItemToWorld")
	void K2_AddItemToWorld(AVoxelWorld* World);

	UFUNCTION(BlueprintNativeEvent, DisplayName = "GetPriority")
	int32 K2_GetPriority() const;
	
	void K2_AddItemToWorld_Implementation(AVoxelWorld* World)
	{
		AddItemToWorld(World);
	}
	int32 K2_GetPriority_Implementation() const
	{
		return GetPriority();
	}

public:
	//~ Begin AVoxelPlaceableItemActor Interface
	virtual void AddItemToWorld(AVoxelWorld* World) {}
	virtual int32 GetPriority() const { return 0; }
	//~ End AVoxelPlaceableItemActor Interface
};