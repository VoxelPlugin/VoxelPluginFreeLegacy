// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"

struct FVoxelChunkUpdate;
struct FVoxelMaterialIndices;

// Fired once per chunk
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdateFinished, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FVoxelOnMaterialInstanceCreated, int32 /*ChunkLOD*/, const FVoxelIntBox& /*ChunkBounds*/, UMaterialInstanceDynamic* /*Instance*/);

class VOXEL_API IVoxelRenderer : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelRendererSubsystemProxy);

	//~ Begin IVoxelSubsystem Interface
	virtual EVoxelSubsystemFlags GetFlags() const override { return EVoxelSubsystemFlags::RecreateRender; }
	//~ End IVoxelSubsystem Interface

	FVoxelOnMaterialInstanceCreated OnMaterialInstanceCreated;

	//~ Begin IVoxelRenderer Interface
	virtual int32 UpdateChunks(
		const FVoxelIntBox& Bounds,
		const TArray<uint64>& ChunksToUpdate,
		const FVoxelOnChunkUpdateFinished& FinishDelegate) = 0;
	virtual void UpdateLODs(uint64 InUpdateIndex, const TArray<FVoxelChunkUpdate>& ChunkUpdates) = 0;

	virtual int32 GetTaskCount() const = 0;
	virtual bool AreChunksDithering() const = 0;

	virtual void ApplyNewMaterials() = 0;
	virtual void ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda) = 0;

	virtual void CreateGeometry_AnyThread(int32 LOD, const FIntVector& ChunkPosition, TArray<uint32>& OutIndices, TArray<FVector>& OutVertices) const = 0;
	//~ End IVoxelRenderer Interface

public:
	UMaterialInterface* GetVoxelMaterial(int32 LOD, const FVoxelMaterialIndices& MaterialIndices) const;
	UMaterialInterface* GetVoxelMaterial(int32 LOD) const;

	void OnMaterialsChanged() const
	{
		// Needed to have errors display again
		UniqueId = UniqueIdCounter++;
	}

private:
	mutable uint64 UniqueId = UniqueIdCounter++;
	static uint64 UniqueIdCounter;
};