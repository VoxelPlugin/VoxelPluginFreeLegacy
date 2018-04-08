// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelRender.h"
#include "VoxelCubicRenderThread.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

class UVoxelProceduralMeshComponent;
class FVoxelQueuedThreadPool;
class FCubicVoxelRender;

class FVoxelCubicChunk
{
public:
	bool bIsValid;

	const FIntVector Position;
	FCubicVoxelRender* const Render;

	TSharedPtr<FAsyncCubicPolygonizerWork> Task;
	UVoxelProceduralMeshComponent* Mesh;

	FVoxelCubicChunk(const FIntVector& Position, FCubicVoxelRender* Render);

	void Tick();
	void Update();


private:
};

/**
 *
 */
class VOXEL_API FCubicVoxelRender : public IVoxelRender
{
public:
	FVoxelQueuedThreadPool* const MeshThreadPool;

	FCubicVoxelRender(AVoxelWorld* World, AActor* ChunksOwner);
	virtual ~FCubicVoxelRender() override;

	virtual void Tick(float DeltaTime) override;
	virtual void AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker) override;

	virtual void UpdateBoxInternal(const FIntBox& Box) override;
	virtual uint8 GetLODAtPosition(const FIntVector& Position) const override;

	UVoxelProceduralMeshComponent* GetMesh(const FIntVector& Position);
	void RemoveMesh(UVoxelProceduralMeshComponent* Mesh);


private:
	TArray<UVoxelProceduralMeshComponent*> InactiveChunks;

	TMap<FIntVector, TSharedRef<FVoxelCubicChunk>> Chunks;
	TArray<TSharedRef<FVoxelCubicChunk>> ChunksArray;
	TArray<TSharedPtr<FAsyncCubicPolygonizerWork>> TasksToDelete;

	TArray<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers;

	float TimeSinceUpdate;

	void UpdateChunks();
};