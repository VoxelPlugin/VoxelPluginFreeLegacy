// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "TransitionDirection.h"
#include "LandscapeGrassType.h"
#include "VoxelThread.h"
#include "VoxelChunk.generated.h"

class VoxelRender;
class ChunkOctree;
class VoxelPolygonizer;

/**
 * Voxel Chunk actor class
 */
UCLASS()
class AVoxelChunk : public AActor
{
	GENERATED_BODY()

public:
	AVoxelChunk();

	/**
	 * Init this
	 * @param	NewPosition		Position of this (Minimal corner)
	 * @param	NewDepth		Width = 16 * 2^Depth
	 * @param	NewWorld		VoxelWorld
	 */
	void Init(TWeakPtr<ChunkOctree> NewOctree);

	/**
	 * Update this for terrain changes
	 * @param	bAsync
	 */
	bool Update(bool bAsync);

	bool UpdateFoliage();

	/**
	 * Check if an adjacent chunk has changed its resolution, and update async if needed
	 */
	void CheckTransitions();

	/**
	 * Schedule destruction of this chunk
	 */
	void Unload();

	/**
	 * Delete this
	 */
	void Delete();

	void SetMaterial(UMaterialInterface* Material);

	bool HasChunkHigherRes(TransitionDirection Direction);

	/**
	* Copy Task section to PrimaryMesh section
	*/
	void OnMeshComplete(FProcMeshSection& InSection);

	void ApplyNewMesh();

	void OnFoliageComplete();

	void ApplyNewFoliage();

private:
	UPROPERTY(EditAnywhere)
		UProceduralMeshComponent* PrimaryMesh;

	UPROPERTY(EditAnywhere)
		TArray<UHierarchicalInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY()
		FProcMeshSection Section;

	FTimerHandle DeleteTimer;

	// ChunkHasHigherRes[TransitionDirection] if Depth != 0
	TArray<bool, TFixedAllocator<6>> ChunkHasHigherRes;

	// Async process tasks
	FAsyncTask<FAsyncPolygonizerTask>* MeshBuilder;
	TArray<FAsyncTask<FAsyncFoliageTask>*> FoliageTasks;

	VoxelPolygonizer* Builder;

	TSharedPtr<ChunkOctree> CurrentOctree;
	VoxelRender* Render;

	uint32 CompletedFoliageTaskCount;

	void OnAllFoliageComplete();

	void DeleteTasks();

	void CreateBuilder();
};