// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "VoxelFoliageSave.h"
#include "VoxelInstancedMeshSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelFoliageBlueprintLibrary.generated.h"

class AVoxelWorld;

UCLASS()
class VOXELFOLIAGE_API UVoxelFoliageBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
		
public:
	// Will replace instanced static mesh instances by actors
	UFUNCTION(BlueprintCallable, Category = "Voxel|Foliage", meta = (DefaultToSelf = "World"))
	static void SpawnVoxelSpawnerActorsInArea(
		TArray<AVoxelFoliageActor*>& OutActors, 
		AVoxelWorld* World,
		FVoxelIntBox Bounds,
		EVoxelSpawnerActorSpawnType SpawnType = EVoxelSpawnerActorSpawnType::OnlyFloating);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Foliage", meta = (DefaultToSelf = "World"))
	static AVoxelFoliageActor* SpawnVoxelSpawnerActorByInstanceIndex(
		AVoxelWorld* World,
		UVoxelHierarchicalInstancedStaticMeshComponent* Component,
		int32 InstanceIndex);

	/**
	 * Add instances to a voxel world foliage system
	 * @param World						The voxel world
	 * @param Transforms				The transforms, relative to the voxel world (but not in voxel space!)
	 * @param CustomData				The custom data to pass to the material. Num = Transforms.Num() * NumCustomDataChannels.
	 *									If NumCustomDataChannels = 3, data would be: Instance0_Data0, Instance0_Data1, Instance0_Data2, Instance1_Data0, Instance1_Data1, ...
	 * @param FloatingDetectionOffset	Increase this if your foliage is enabling physics too soon
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Foliage", meta = (DefaultToSelf = "World", AdvancedDisplay = "FloatingDetectionOffset"))
	static void AddInstances(
		AVoxelWorld* World,
		const TArray<FTransform>& Transforms,
		const TArray<float>& CustomData,
		FVoxelInstancedMeshKey MeshKey,
		FVector FloatingDetectionOffset = FVector(0, 0, -10));
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static FVoxelFoliageSave GetSpawnersSave(AVoxelWorld* World);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Spawners", meta = (DefaultToSelf = "World"))
	static void LoadFromSpawnersSave(AVoxelWorld* World, const FVoxelFoliageSave& Save);
};