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

	int GetValue(FIntVector position);
	FColor GetColor(FIntVector position);

	void SetValue(FIntVector position, int value);
	void SetColor(FIntVector position, FColor color);

	bool IsInWorld(FIntVector position);

	void Update(FIntVector position);
	void ScheduleUpdate(FIntVector position);

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
		void Update(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ScheduleUpdate(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyQueuedUpdates();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateCameraPosition(FVector position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FVector position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetValue(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FColor GetColor(FVector position);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetValue(FVector position, int value);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetColor(FVector position, FColor color);

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
