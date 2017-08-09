// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkSaveStruct.h"
#include "VoxelWorldGenerator.h"
#include "QueuedThreadPool.h"
#include "Camera/PlayerCameraManager.h"
#include "VoxelWorld.generated.h"


class ChunkOctree;
class VoxelData;
class AVoxelChunk;

DECLARE_LOG_CATEGORY_EXTERN(VoxelLog, Log, All);
DECLARE_STATS_GROUP(TEXT("Voxels"), STATGROUP_Voxel, STATCAT_Advanced);

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()
public:
	AVoxelWorld();

	int Size();

	float GetDeletionDelay();
	float GetQuality();
	float GetHighResolutionDistanceOffset();
	bool GetRebuildBorders();

	AVoxelChunk* GetChunkAt(FIntVector Position);
	void QueueUpdate(TWeakPtr<ChunkOctree> Chunk);

	FQueuedThreadPool* ThreadPool;

public:
	// Material to use
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;


	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FIntVector GlobalToLocal(FVector Position);

	/**
	 * Add
	 * @param	Position	Position in voxel space
	 * @param	Strength	Amount to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FIntVector Position, int Strength);
	/**
	 * Remove
	 * @param	Position	Position in voxel space
	 * @param	Strength	Amount to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Remove(FIntVector Position, int Strength);

	/**
	 * Update chunk
	 * @param	Position	Position in voxel space
	 * @param	bAsync		Async?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Update(FIntVector Position, bool bAsync = true);
	/**
	 * Add chunk to update queue
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void QueueUpdate(FIntVector Position);
	/**
	 * Update all chunks in queue
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void ApplyQueuedUpdates(bool bAsync = true);

	/**
	 * Update all chunks
	 * @param	bAsync	Async?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateAll(bool bAsync = true);

	/**
	 * Update camera position for LODs
	 * @param	Position	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateCameraPosition(FVector Position);

	/**
	 * Is position in this world?
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FIntVector Position);


	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetValue(FIntVector Position);
	/**
	 * Get color at position
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FColor GetColor(FIntVector Position);

	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value		int between -127 and 127
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetValue(FIntVector Position, int Value);
	/**
	 * Set color at position
	 * @param	Position	Position in voxel space
	 * @param	Color		FColor
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetColor(FIntVector Position, FColor Color);

	/**
	 * Get array to save world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		TArray<FVoxelChunkSaveStruct> GetSaveArray();
	/**
	 * Load world from array
	 * @param	SaveArray	Array to load from
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	// Lock Depth and VoxelMaterial when in play
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

private:
	// Width = 16 * 2^Depth
	UPROPERTY(EditAnywhere, Category = Voxel)
		int Depth;
	// Time to wait before deleting old chunks to avoid holes
	UPROPERTY(EditAnywhere, Category = Voxel)
		float DeletionDelay;
	// Factor for LODs
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0.01", ClampMax = "10", UIMin = "0.01", UIMax = "10"))
		float Quality;
	// Distance where chunks must be at the highest resolution
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0"))
		float HighResolutionDistanceOffset;
	// If disabled, holes may appear between far chunks but can improve performance when moving
	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bRebuildBorders;

	// Generator for this world
	UPROPERTY(EditAnywhere, Category = Voxel)
		TSubclassOf<UVoxelWorldGenerator> WorldGenerator;

	// Camera to set LODs
	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		APlayerCameraManager* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bAutoFindCamera;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bAutoUpdateCameraPosition;

private:
	TSharedPtr<ChunkOctree> MainOctree;
	TSharedPtr<VoxelData> Data;

	bool bNotCreated;

	TArray<TWeakPtr<ChunkOctree>> ChunksToUpdate;

	UPROPERTY()
		UVoxelWorldGenerator* WorldGeneratorInstance;
};
