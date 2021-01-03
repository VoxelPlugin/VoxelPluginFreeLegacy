// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticBitArray.h"
#include "VoxelRender/Meshers/VoxelMesher.h"

#define CUBIC_CHUNK_SIZE_WITH_NEIGHBORS (RENDER_CHUNK_SIZE + 2)

class FVoxelConstDataAccelerator;

class FVoxelGreedyCubicMesher : public FVoxelMesher
{
public:
	using FVoxelMesher::FVoxelMesher;
	
protected:
	virtual FVoxelIntBox GetBoundsToCheckIsEmptyOn() const override final;
	virtual FVoxelIntBox GetBoundsToLock() const override final;
	virtual TVoxelSharedPtr<FVoxelChunkMesh> CreateFullChunkImpl(FVoxelMesherTimes& Times) override final;
	virtual void CreateGeometryImpl(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<FVector>& Vertices) override final;

private:
	TUniquePtr<FVoxelConstDataAccelerator> Accelerator;
	
	template<typename T>
	void CreateGeometryTemplate(FVoxelMesherTimes& Times, TArray<uint32>& Indices, TArray<T>& Vertices, TArray<FColor>* TextureData, TArray<FVoxelIntBox>* CollisionCubes);
	
	struct FCubicQuad
	{
		uint32 Layer;
		uint32 StartX;
		uint32 StartY;
		uint32 SizeX;
		uint32 SizeY;
	};

	template<uint32 GridSize, typename Allocator>
	void GreedyMeshing2D(TVoxelStaticBitArray<GridSize * GridSize * GridSize>& InFaces, TArray<FCubicQuad, Allocator>& OutQuads);

	template<uint32 GridSize, typename Allocator>
	void GreedyMeshing3D(TVoxelStaticBitArray<GridSize * GridSize * GridSize>& Data, TArray<FVoxelIntBox, Allocator>& OutCubes);

public:
	template<typename T>
	void AddFace(
		FVoxelMesherTimes& Times, 
		int32 Direction,
		const FCubicQuad& Quad,
		int32 Step,
		TArray<uint32>& Indices,
		TArray<T>& Vertices,
		TArray<FColor>* TextureData);
};
