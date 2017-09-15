// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelSave.h"
#include "VoxelWorldGenerator.h"
#include "QueuedThreadPool.h"
#include "Camera/PlayerCameraManager.h"
#include "VoxelLODProfile.h"
#include "VoxelMaterial.h"
#include "VoxelWorld.generated.h"


class ChunkOctree;
class ValueOctree;
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
	~AVoxelWorld();

	int Size() const;

	float GetDeletionDelay() const;
	float GetLODToleranceZone() const;
	bool GetRebuildBorders() const;
	float GetLODAt(float Distance) const;
	FQueuedThreadPool* GetThreadPool();
	VoxelData* GetData();
	bool IsCreated() const;

	TSharedPtr<ChunkOctree> GetChunkOctree() const;

	TSharedPtr<ValueOctree> GetValueOctree() const;

	AVoxelChunk* GetChunkAt(FIntVector Position) const;

	FColor GetColor(FIntVector Position) const;

	void QueueUpdate(TWeakPtr<ChunkOctree> Chunk, bool bAsync);

	void Load();
	void Unload();


	UPROPERTY(EditAnywhere, Category = Voxel)
		TSubclassOf<UVoxelLODProfile> LODProfile;

	// Material to use
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bDebugMultiplayer;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bDrawChunksBorders;


	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FIntVector GlobalToLocal(FVector Position) const;

	/**
	 * Add
	 * @param	Position	Position in voxel space
	 * @param	Value		Value to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Add(FIntVector Position, float Value);

	/**
	 * Add chunk to update queue that will be processed at the end of the frame
	 * @param	Position	Position in voxel space
	 * @param	bAsync		Async update?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void QueueUpdate(FIntVector Position, bool bAsync);

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
	 * @return	IsInWorld?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsInWorld(FIntVector Position) const;


	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 * @return	Value at position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		float GetValue(FIntVector Position) const;
	/**
	 * Get material at position
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FVoxelMaterial GetMaterial(FIntVector Position) const;

	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value		Value to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetValue(FIntVector Position, float Value);
	/**
	 * Set material at position
	 * @param	Position	Position in voxel space
	 * @param	Material	FVoxelMaterial
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void SetMaterial(FIntVector Position, FVoxelMaterial Material);

	/**
	 * Get array to save world
	 * @return	SaveArray
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FVoxelWorldSave GetSave() const;
	/**
	 * Load world from save
	 * @param	Save	Save to load from
	 * @param	bReset	Reset existing world? Set to false only if current world is unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void LoadFromSave(FVoxelWorldSave Save, bool bReset = true);

	/**
	 * Sync world over network
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void Sync();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	// Lock Depth and VoxelMaterial when in play
	bool CanEditChange(const UProperty* InProperty) const override;
#endif

private:
	// Width = 16 * 2^Depth
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0", ClampMax = "9", UIMin = "0", UIMax = "9"))
		int Depth;
	// Multiplayer game?
	UPROPERTY(EditAnywhere, Category = Voxel)
		bool bMultiplayer;
	// FPS of the multiplayer sync
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0.01", ClampMax = "120", UIMin = "0.01", UIMax = "120"))
		float MultiplayerFPS;
	// Time to wait before deleting old chunks to avoid holes
	UPROPERTY(EditAnywhere, Category = Voxel)
		float DeletionDelay;
	// Chunk at distance D has to verify: LODCurve(D) - LODCurve(D - Width * LODToleranceZone) < LOD < LODCurve(D) + LODCurve(D + Width * LODToleranceZone)
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0.01", ClampMax = "1", UIMin = "0.01", UIMax = "1"))
		float LODToleranceZone;
	// If disabled, holes may appear between far chunks but can improve performance when moving
	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bRebuildBorders;

	// Generator for this world
	UPROPERTY(EditAnywhere, Category = Voxel)
		TSubclassOf<AVoxelWorldGenerator> WorldGenerator;

	// Camera to set LODs
	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		APlayerCameraManager* PlayerCamera;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bAutoFindCamera;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bAutoUpdateCameraPosition;

	/**
	 * Load server world
	 * @param	ValueDiffArray	Values diff array
	 * @param	ColorDiffArray	Colors diff array
	 */
	UFUNCTION(NetMulticast, Reliable)
		void MulticastLoadArray(const TArray<FVoxelValueDiff>& ValueDiffArray, const TArray<FVoxelColorDiff>& ColorDiffArray);

	// Instanced world generator
	UPROPERTY()
		AVoxelWorldGenerator* InstancedWorldGenerator;

	// Is VoxelWorld created?
	bool bIsCreated;

	// Elapsed time since last sync
	float TimeSinceSync;

	// Chunks waiting for update
	TSet<TWeakPtr<ChunkOctree>> QueuedChunks;
	// Chunk that needs to be updated synchronously
	TSet<uint64> ChunksToUpdateSynchronously;



	TSharedPtr<ChunkOctree> MainChunkOctree; // Shared ptr because each ChunkOctree need a reference to it, and Main isn't the child of anything

	FQueuedThreadPool* ThreadPool;

	VoxelData* Data;

	/**
	* Update all chunks in queue
	*/
	void ApplyQueuedUpdates();
};
