// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class ChunkOctree;
class VoxelData;
class AVoxelChunk;

DECLARE_LOG_CATEGORY_EXTERN(VoxelWorldLog, Log, All);

UCLASS()
class PROCEDURAL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
public:
	AVoxelWorld();
	~AVoxelWorld();

	int Size();

	int GetDepthAt(FIntVector position);
	void ScheduleUpdate(TWeakPtr<ChunkOctree> chunk);
public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;
	UPROPERTY(EditAnywhere, Category = Voxel)
		int Depth;
	UPROPERTY(EditAnywhere, Category = Voxel)
		float CollisionFPS;


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FVector hitPoint, FVector normal, float range = 1, int strength = 1);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Remove(FVector hitPoint, FVector normal, float range = 1, int strength = 1);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Update(FIntVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ScheduleUpdate(FIntVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyQueuedUpdates();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateCameraPosition(FVector position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FIntVector position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetValue(FIntVector position);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Lock Depth and VoxelMaterial when in play
	virtual bool CanEditChange(const UProperty* InProperty) const override;

private:
	ChunkOctree* MainOctree;
	VoxelData* Data;

	bool bNotCreated;

	TArray<TWeakPtr<ChunkOctree>> ChunksToUpdate;

	void ModifyVoxel(FVector hitPoint, FVector normal, float range, int strength, bool add);
};
