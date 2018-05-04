// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "IntBox.h"
#include "VoxelSave.h"
#include "VoxelWorldGenerator.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "VoxelRenderFactory.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "PhysicsEngine/BodyInstance.h"
#include "VoxelWorld.generated.h"

class IVoxelRender;
class FVoxelData;
class FVoxelActorOctree;
class UVoxelInvokerComponent;
class AVoxelWorldEditorInterface;
class AVoxelActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientConnection);

/**
 * Voxel World actor class
 */
UCLASS()
class VOXEL_API AVoxelWorld : public AActor
{
	GENERATED_BODY()

public:
	// Used to get a ref to AVoxelWorldEditor::StaticClass()
	UClass* VoxelWorldEditorClass;

	UPROPERTY(BlueprintAssignable)
	FOnClientConnection OnClientConnection;
		
	AVoxelWorld();
	~AVoxelWorld();
	

	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void CreateWorld();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void DestroyWorld();

	/**
	 * Create the world when in editor (not PIE)
	 */
	void CreateInEditor();
	/**
	 * Destroy the world when in editor (not PIE)
	 */
	void DestroyInEditor();

	/**
	 * Add a VoxelInvoker
	 */
	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

	// Getters
	FORCEINLINE AVoxelWorldEditorInterface* GetVoxelWorldEditor() const;
	FORCEINLINE FVoxelData* GetData() const;
	FORCEINLINE FVoxelWorldGeneratorInstance* GetWorldGenerator() const;
	FORCEINLINE int GetMaxCollisionsLOD() const;
	FORCEINLINE bool GetDebugCollisions() const;
	FORCEINLINE float GetCollisionsUpdateRate() const;
	FORCEINLINE float GetLODUpdateRate() const;
	FORCEINLINE float GetChunksFadeDuration() const;
	FORCEINLINE int GetCollisionsThreadCount() const;
	FORCEINLINE int GetMeshThreadCount() const;
	FORCEINLINE FQueuedThreadPool* GetAsyncTasksThreadPool() const;
	FORCEINLINE bool GetCreateAdditionalVerticesForMaterialsTransitions() const;
	UMaterialInstanceDynamic* GetVoxelMaterialDynamicInstance();
	FORCEINLINE UMaterialInterface* GetVoxelMaterial() const;
	FORCEINLINE bool GetEnableNormals() const;
	FORCEINLINE int GetLODLimit() const;
	FORCEINLINE const FBodyInstance& GetCollisionPresets() const;
	FORCEINLINE UVoxelWorldSaveObject* GetSaveObject() const;

	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int GetSeed() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetSeed(int Seed);

	/**
	 * Set the voxel material
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetVoxelMaterial(UMaterialInterface* NewMaterial);

	/**
	 * Set the world LOD. The world must not be created
	 */	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetLOD(uint8 NewLOD);
	
	/**
	 * Replace the World Generator. Can be called at runtime
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetWorldGenerator(UVoxelWorldGenerator* NewGenerator);
	
	/**
	 * Swap the world generator, keeping a sphere unmodified. Need to call UpdateAll after this
	 * @param	NewGenerator	The new generator outside the sphere
	 * @param	Center			The center of the sphere, in voxels
	 * @param	Radius			The radius of the sphere, in voxels
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	 void SwapWorldGeneratorSphere(UVoxelWorldGenerator* NewGenerator, const FIntVector& Center, float Radius);

	/**
	 * Is this world created?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsCreated() const;

	/**
	 * Get the render chunks LOD at Position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	int GetLODAt(const FIntVector& Position) const;
	
	/**
	 * Get the world LOD
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	uint8 GetLOD() const;

	/**
	 *  Size of a voxel in unit (cm)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	float GetVoxelSize() const;

	/**
	 * Bounds of this world
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntBox GetBounds() const;

	/**
	 * Convert position from world space to voxel space
	 * @param	Position	Position in world space
	 * @return	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FIntVector GlobalToLocal(const FVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector GlobalToLocalFloat(const FVector& Position) const;

	/**
	 * Convert position from voxel space to world space
	 * @param	Position	Position in voxel space
	 * @return	Position in world space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobal(const FIntVector& Position) const;
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector LocalToGlobalFloat(const FVector& Position) const;

	/**
	 * Get the 8 neighbors in voxel space of GlobalPosition
	 * @param	GlobalPosition	The position in world space
	 * @return	The 8 neighbors in voxel space 
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	TArray<FIntVector> GetNeighboringPositions(const FVector& GlobalPosition) const;

	/**
	 * Add chunks at position to update queue
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksAtPosition(const FIntVector& Position);

	/**
	 * Add chunks overlapping box to update queue
	 * @param	Box			Box
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateChunksOverlappingBox(const FIntBox& Box);

	/**
	 * Update all the chunks
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void UpdateAll();

	/**
	 * Is position in this world?
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsInWorld(const FIntVector& Position) const;

	/**
	 * Get the intersection using voxel data. Doesn't depend on LOD. Useful for short distances, but costful for big ones
	 * @param	Start				The start of the raycast. The start and the end must have only one coordinate not in common
	 * @param	End					The end of the raycast. The start and the end must have only one coordinate not in common
	 * @return	GlobalPosition		The world position of the intersection if found
	 * @return	VoxelPosition		The voxel position of the intersection if found
	 * @return	Has intersected?
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Voxel", meta = (DisplayName = "Get Intersection"))
	bool GetIntersection(const FIntVector& Start, const FIntVector& End, FVector& GlobalPosition, FIntVector& VoxelPosition) const;

	/**
	 * Get the normal at the voxel position Position using gradient. May differ from the mesh normal
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVector GetNormal(const FIntVector& Position) const;

	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 * @return	Value at position
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	float GetValue(const FIntVector& Position) const;
	/**
	 * Get material at position
	 * @param	Position	Position in voxel space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	FVoxelMaterial GetMaterial(const FIntVector& Position) const;

	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value		Value to set
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetValue(const FIntVector& Position, float Value);
	/**
	 * Set material at position
	 * @param	Position	Position in voxel space
	 * @param	Material	FVoxelMaterial
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void SetMaterial(const FIntVector& Position, const FVoxelMaterial& Material);
	

	
	/**
	 * Is the position inside the meshes? Not accurate
	 * @param	Position	Position in global space
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	bool IsInside(const FVector& Position);

	/**
	 * Get the world save
	 * @return	SaveArray
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	void GetSave(FVoxelWorldSave& OutSave) const;
	/**
	 * Load world from save
	 * @param	Save	Save to load from
	 * @param	bReset	Reset existing world? Set to false only if current world is unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel", meta = (AdvancedDisplay = "1"))
	void LoadFromSave(const FVoxelWorldSave& Save, bool bReset = true);
	

protected:
	//~ Begin AActor Interface
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void Tick(float DeltaTime) override;
#if WITH_EDITOR
	bool ShouldTickIfViewportsOnly() const override;
	bool CanEditChange(const UProperty* InProperty) const override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End AActor Interface

private:
	// WorldSizeInVoxel = CHUNK_SIZE * 2^Depth. Has little impact on performance
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1", ClampMax = "19", UIMin = "1", UIMax = "19", DisplayName = "Octree Depth"))
	int LOD;

	// Size of an edge of the world
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1"))
	int WorldSizeInVoxel;
	
	// Chunks can't have a LOD higher than this. Useful is background has a too low resolution. WARNING: Don't set this too low, 5 under LOD should be safe
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1", ClampMax = "19", UIMin = "1", UIMax = "19"))
	int LODLimit;

	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	float VoxelSize;

	// Generator of this world
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	FVoxelWorldGeneratorPicker WorldGenerator;

	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	UVoxelWorldSaveObject* SaveObject;

	// The seed of this world. For now only used for grass
	UPROPERTY(EditAnywhere, Category = "Voxel|General", meta = (ClampMin = "1", UIMin = "1"))
	int32 Seed;

	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	bool bCreateWorldAutomatically;

	// Keep all the changes in memory to enable undo/redo. Can be expensive
	UPROPERTY(EditAnywhere, Category = "Voxel|General")
	bool bEnableUndoRedo;



	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	EVoxelRenderType RenderType;

	// The material of the world
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering")
	UMaterialInterface* VoxelMaterial;
	
	
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", AdvancedDisplay)
	bool bEnableNormals;

	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", meta = (ClampMin = "0", UIMin = "0"), AdvancedDisplay)
	float ChunksFadeDuration;

	// Add vertices to reduce materials transitions glitches, however it can make holes in the grounds if tessellation is enabled.
	UPROPERTY(EditAnywhere, Category = "Voxel|Rendering", AdvancedDisplay)
	bool bCreateAdditionalVerticesForMaterialsTransitions;



	// Max LOD to compute collisions on. Inclusive. Collisions around player are always computed
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions", AdvancedDisplay, meta = (ClampMin = "-1", UIMin = "-1"))
	int MaxCollisionsLOD;

	// Should the collisions meshes around the player be rendered?
	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions", AdvancedDisplay)
	bool bDebugCollisions;

	UPROPERTY(EditAnywhere, Category = "Voxel|Collisions", AdvancedDisplay)
	FBodyInstance CollisionPresets;


	// Number of collision update per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", AdvancedDisplay, meta = (ClampMin = "0.001", UIMin = "0.001"))
	float CollisionsUpdateRate;

	// Number of LOD update per second
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", AdvancedDisplay, meta = (ClampMin = "0.001", UIMin = "0.001"), DisplayName = "LOD Update Rate")
	float LODUpdateRate;

	// Number of threads allocated for the mesh processing. Setting it too high may impact performance
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = "1", UIMin = "1"), AdvancedDisplay)
	int MeshThreadCount;

	// Number of threads allocated for the collisions meshes processing. Setting it too low may impact performance
	UPROPERTY(EditAnywhere, Category = "Voxel|Performance", meta = (ClampMin = "1", UIMin = "1"), AdvancedDisplay)
	int CollisionsThreadCount;





	TSharedPtr<FVoxelWorldGeneratorInstance> InstancedWorldGenerator;

	UPROPERTY()
	AVoxelWorldEditorInterface* VoxelWorldEditor;

	UPROPERTY()
	class AVoxelChunksOwner* ChunksOwner;

	UPROPERTY()
	UMaterialInstanceDynamic* VoxelMaterialInstance;

	

	FQueuedThreadPool* AsyncTasksThreadPool;
	

	TSharedPtr<FVoxelData> Data;
	TSharedPtr<IVoxelRender> Render;


	bool bIsCreated;


	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;

	// Create the world
	void CreateWorldInternal(AActor* ChunksOwner = nullptr);
	// Destroy the world
	void DestroyWorldInternal();

};
