// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelIntBox.h"
#include "VoxelPhysicsPartSpawnerInterface.h"
#include "VoxelPhysics.generated.h"

class FVoxelData;
class AVoxelWorld;


UCLASS()
class VOXEL_API UVoxelPhysicsTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
public:
	/**
	 * Apply voxel physics in a section of the voxel world by removing floating parts
	 * @param Results				Each part spawner result correspond to an individual floating part. Cast them to their corresponding class to get info from them.
	 * @param World					The voxel world
	 * @param Bounds				The bounds to search in (caution: keep this small!)
	 * @param PartSpawner			The part spawner that will handle the spawning of new parts (construct a new object of class VoxelPhysicsPartSpawner_Smthg)
	 *								If null will just remove the floating parts
	 * @param MinParts				The minimum number of parts (inclusive) to have before removing them. This is useful to avoid considering the whole world as a floating part
	 * @param bHideLatentWarnings	Hide latent warnings
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (DefaultToSelf = "World", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "MinParts, bDebug, bHideLatentWarnings"))
	static void ApplyVoxelPhysics(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<TScriptInterface<IVoxelPhysicsPartSpawnerResult>>& Results,
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		TScriptInterface<IVoxelPhysicsPartSpawner> PartSpawner,
		int32 MinParts = 2,
		bool bDebug = false,
		bool bHideLatentWarnings = false);
};