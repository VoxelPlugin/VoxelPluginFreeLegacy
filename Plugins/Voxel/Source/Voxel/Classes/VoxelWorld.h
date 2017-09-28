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
#include "LandscapeGrassType.h"
#include "VoxelWorld.generated.h"

using namespace UP;
using namespace UM;
using namespace US;
using namespace UC;

class ChunkOctree;
class ValueOctree;
class VoxelData;
class AVoxelChunk;
class UVoxelInvokerComponent;

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

	float GetDeletionDelay() const;
	bool GetRebuildBorders() const;
	FQueuedThreadPool* GetThreadPool();
	VoxelData* GetData();
	bool IsCreated() const;

	TSharedPtr<ChunkOctree> GetChunkOctree() const;

	TSharedPtr<ValueOctree> GetValueOctree() const;

	AVoxelChunk* GetChunkAt(FIntVector Position) const;

	FColor GetColor(FIntVector Position) const;

	void QueueUpdate(TWeakPtr<ChunkOctree> Chunk, bool bAsync);

	void AddChunkToPool(AVoxelChunk* Chunk);
	AVoxelChunk* GetChunkFromPool();

	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	void Load();
	void Unload();

	UPROPERTY(EditAnywhere, Category = Voxel)
		TArray<ULandscapeGrassType*> GrassTypes;

	// Hack
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (EditCondition = "bDebugMultiplayer"))
		float GrassFPS;

	// Grass max depth (inclusive)
	UPROPERTY(EditAnywhere, Category = Voxel)
		int MaxGrassDepth;

	// Material to use
	UPROPERTY(EditAnywhere, Category = Voxel)
		UMaterialInterface* VoxelMaterial;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bDebugMultiplayer;

	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bDrawChunksBorders;

	// Size of this world
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int Size() const;

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
	UPROPERTY(EditAnywhere, Category = Voxel, meta = (ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20"))
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

	// If disabled, holes may appear between far chunks but can improve performance when moving
	UPROPERTY(EditAnywhere, Category = Voxel, AdvancedDisplay)
		bool bRebuildBorders;

	// Generator for this world
	UPROPERTY(EditAnywhere, Category = Voxel)
		TSubclassOf<AVoxelWorldGenerator> WorldGenerator;

	/**
	 * Sync world
	 * @param	ValueDiffArray	Values diff array
	 * @param	ColorDiffArray	Colors diff array
	 */
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSyncValues(const TArray<FVoxelValueDiff>& ValueDiffArray);
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSyncColors(const TArray<FVoxelColorDiff>& ColorDiffArray);

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

	// List of unused chunks. Allows reusing of chunks
	std::forward_list<AVoxelChunk*> ChunksPool;

	std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> VoxelInvokerComponents;

	/**
	* Update all chunks in queue
	*/
	void ApplyQueuedUpdates();
};
