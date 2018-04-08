// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

class UVoxelInvokerComponent;
class UVoxelChunkComponent;
class FVoxelChunkOctree;
class AVoxelWorld;

class VOXEL_API IVoxelRender
{
public:
	AVoxelWorld* const World;
	AActor* const ChunksOwner;

	IVoxelRender(AVoxelWorld* World, AActor* ChunksOwner)
		: World(World)
		, ChunksOwner(ChunksOwner)
	{

	}
	virtual ~IVoxelRender() = default;

	virtual void Tick(float DeltaTime) = 0;
	virtual void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker) = 0;
	
	void UpdateBox(const FIntBox& Box) { UpdateBoxInternal(FIntBox(Box.Min - FIntVector(2, 2, 2), Box.Max + FIntVector(2, 2, 2))); }
	virtual void UpdateBoxInternal(const FIntBox& Box) = 0;

	virtual uint8 GetLODAtPosition(const FIntVector& Position) const = 0;
};