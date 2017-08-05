// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkSaveStruct.h"
#include "VoxelWorld.generated.h"


class ChunkOctree;
class VoxelData;
class AVoxelChunk;

DECLARE_LOG_CATEGORY_EXTERN(VoxelWorldLog, Log, All);
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

	int GetDepthAt(FIntVector position);
	void ScheduleUpdate(TWeakPtr<ChunkOctree> chunk);

	FQueuedThreadPool* ThreadPool;

public:
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalAdd(FVector position, int strength);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FIntVector position, int strength);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalRemove(FVector position, int strength);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Remove(FIntVector position, int strength);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalUpdate(FVector position, bool async = true);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Update(FIntVector position, bool async = true);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalScheduleUpdate(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ScheduleUpdate(FIntVector position);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyQueuedUpdates(bool async = true);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateAll(bool async = true);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateCameraPosition(FVector position);



	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool GlobalIsInWorld(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FIntVector position);



	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GlobalGetValue(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetValue(FIntVector position);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FColor GlobalGetColor(FVector position);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FColor GetColor(FIntVector position);


	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalSetValue(FVector position, int value);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetValue(FIntVector position, int value);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void GlobalSetColor(FVector position, FColor color);
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetColor(FIntVector position, FColor color);

	//UFUNCTION(BlueprintCallable, Category = "Voxel")
	TArray<FVoxelChunkSaveStruct> GetSaveArray();
	//UFUNCTION(BlueprintCallable, Category = "Voxel")
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> saveArray);

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

private:
	TSharedPtr<ChunkOctree> MainOctree;
	TSharedPtr<VoxelData> Data;

	bool bNotCreated;

	TArray<TWeakPtr<ChunkOctree>> ChunksToUpdate;
};
