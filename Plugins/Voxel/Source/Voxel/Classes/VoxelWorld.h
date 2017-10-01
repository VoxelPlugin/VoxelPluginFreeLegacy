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

class FVoxelRender;
class FVoxelData;
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

	void Load();
	void Unload();

	FVoxelData* Data;
	FVoxelRender* Render;


	// Width = 16 * 2^Depth
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMin = "0", ClampMax = "20", UIMin = "0", UIMax = "20"))
		int Depth;

	// Time to wait before deleting old chunks to avoid holes
	UPROPERTY(EditAnywhere, Category = "Voxel")
		float DeletionDelay;

	UPROPERTY(EditAnywhere, Category = "Voxel", AdvancedDisplay)
		bool bComputeTransitions;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		bool IsCreated() const;

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		int GetDepthAt(FIntVector Position) const;

	UPROPERTY(EditAnywhere, Category = "Voxel")
		TArray<ULandscapeGrassType*> GrassTypes;

	UPROPERTY(EditAnywhere, Category = "Voxel")
		float FoliageFPS;

	UPROPERTY(EditAnywhere, Category = "Voxel")
		float MeshFPS;

	UPROPERTY(EditAnywhere, Category = "Voxel")
		UMaterialInterface* VoxelMaterial;


	// Size of a voxel in cm
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		float GetVoxelSize();

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

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		FVector LocalToGlobal(FIntVector Position) const;

	/**
	 * Add chunk to update queue that will be processed at the end of the frame
	 * @param	Position	Position in voxel space
	 * @param	bAsync		Async update?
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateChunksAtPosition(FIntVector Position, bool bAsync);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
		void UpdateAll(bool bAsync);

	void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker);

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

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void Tick(float DeltaTime) override;

private:

	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, Category = "Voxel", AdvancedDisplay)
		float VoxelSize;

	UPROPERTY(EditAnywhere, Category = "Voxel", AdvancedDisplay)
		int MeshThreadCount;

	UPROPERTY(EditAnywhere, Category = "Voxel", AdvancedDisplay)
		int FoliageThreadCount;

	// Generator for this world
	UPROPERTY(EditAnywhere, Category = "Voxel")
		TSubclassOf<AVoxelWorldGenerator> WorldGenerator;

	// Instanced world generator
	UPROPERTY()
		AVoxelWorldGenerator* InstancedWorldGenerator;

	UPROPERTY()
		bool bIsCreated;
};
