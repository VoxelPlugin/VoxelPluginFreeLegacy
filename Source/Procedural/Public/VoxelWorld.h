// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorld.generated.h"

class ChunkOctree;
class VoxelData;

UCLASS()
class PROCEDURAL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
public:
	AVoxelWorld();
	~AVoxelWorld();

	signed char GetValue(int x, int y, int z);
	virtual bool CanEditChange(const UProperty* InProperty) const override;


	TArray<ChunkOctree*> ChunksToUpdate;
public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;
	UPROPERTY(EditAnywhere, Category = Voxel)
		int Depth;


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FVector hitPoint, FVector normal, float range);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Remove(FVector hitPoint, FVector normal, float range);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Update(int x, int y, int z);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ScheduleUpdate(int x, int y, int z);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyUpdate();


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(int x, int y, int z);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	ChunkOctree* MainOctree;
	VoxelData* Data;
	bool bNotCreated;
};
