// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkSaveStruct.h"
#include "VoxelWorld.generated.h"


class ChunkOctree;
class VoxelData;
class AVoxelChunk;

DECLARE_LOG_CATEGORY_EXTERN(VoxelLog, Log, All);
DECLARE_STATS_GROUP(TEXT("Voxels"), STATGROUP_Voxel, STATCAT_Advanced);

UCLASS()
class PROCEDURAL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
public:
	AVoxelWorld();
	~AVoxelWorld();

	int Size();

	float GetDeletionDelay();
	float GetQuality();
	bool GetRebuildBorders();

	AVoxelChunk* GetChunkAt(FIntVector Position);
	void ScheduleUpdate(TWeakPtr<ChunkOctree> Chunk);

	FQueuedThreadPool* ThreadPool;

public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FIntVector GlobalToLocal(FVector Position);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FIntVector Position, int Strength);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Remove(FIntVector Position, int Strength);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Update(FIntVector Position, bool Async = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ScheduleUpdate(FIntVector Position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyQueuedUpdates(bool Async = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateAll(bool Async = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateCameraPosition(FVector Position);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FIntVector Position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetValue(FIntVector Position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FColor GetColor(FIntVector Position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetValue(FIntVector Position, int Value);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetColor(FIntVector Position, FColor Color);

	//UFUNCTION(BlueprintCallable, Category = "Voxel")
	TArray<FVoxelChunkSaveStruct> GetSaveArray();
	//UFUNCTION(BlueprintCallable, Category = "Voxel")
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Lock Depth and VoxelMaterial when in play
	virtual bool CanEditChange(const UProperty* InProperty) const override;

private:
	UPROPERTY(EditAnywhere, Category = Voxel)
		int Depth;
	UPROPERTY(EditAnywhere, Category = Voxel)
		float CollisionFPS;
	UPROPERTY(EditAnywhere, Category = Voxel)
		float DeletionDelay;
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0.01", ClampMax = "10", UIMin = "0.01", UIMax = "10"))
		float Quality;
	UPROPERTY(EditAnywhere, Category = Voxel)
		bool bRebuildBorders;

private:
	TSharedPtr<ChunkOctree> MainOctree;
	TSharedPtr<VoxelData> Data;

	bool bNotCreated;

	TArray<TWeakPtr<ChunkOctree>> ChunksToUpdate;
};
