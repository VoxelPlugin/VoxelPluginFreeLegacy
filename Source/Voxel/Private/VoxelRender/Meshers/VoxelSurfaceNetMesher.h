// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelRender/Meshers/VoxelMesher.h"

#define SN_CHUNK_SIZE (RENDER_CHUNK_SIZE + 1) /* +1 since SN vertices are within cells */
#define SN_EXTENDED_CHUNK_SIZE (RENDER_CHUNK_SIZE + 3) /* +3 to get parent's outer edge */

class FVoxelSurfaceNetMesher : public FVoxelMesher
{
public:
	using FVoxelMesher::FVoxelMesher;

protected:
	virtual FIntBox GetBoundsToCheckIsEmptyOn() const override final;
	virtual FIntBox GetBoundsToLock() const override final;
	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) override final;
	virtual void CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices) override final;
	
private:
	TUniquePtr<FVoxelConstDataAccelerator> Accelerator;

	FVoxelValue CachedValues[SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE];
	float EdgeFactors[SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * SN_EXTENDED_CHUNK_SIZE * 3]; // edge blending factors for each cell, X,Y,Z
	uint32 VertexIndices[SN_CHUNK_SIZE * SN_CHUNK_SIZE * SN_CHUNK_SIZE]; // final vertex indices, per voxel. 65535 if no vertex
	uint8 VertexSNCases[SN_CHUNK_SIZE * SN_CHUNK_SIZE * SN_CHUNK_SIZE]; // surface net voxel cases for each cell
	
	template<typename TVertex>
	void CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<TVertex>& Vertices);
};