// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRendererMeshHandler.h"

class FVoxelRendererClusteredMeshHandler;
class FVoxelRendererBasicMeshHandler;

class FVoxelRendererMixedMeshHandler : public IVoxelRendererMeshHandler
{
public:
	explicit FVoxelRendererMixedMeshHandler(IVoxelRenderer& Renderer);
	virtual ~FVoxelRendererMixedMeshHandler() override;
	
	//~ Begin IVoxelRendererMeshHandler Interface
	virtual FChunkId AddChunkImpl(int32 LOD, const FIntVector& Position) override;
	virtual void ApplyAction(const FAction& Action) override;
	virtual void ClearChunkMaterials() override;
	virtual void Tick(double MaxTime) override;

	virtual void RecomputeMeshPositions() override;
	virtual void ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda) override;
	virtual void StartDestroying() override;
	//~ End IVoxelRendererMeshHandler Interface

private:
	// Need to be shared ref, else their AsShared fail
	TVoxelSharedRef<FVoxelRendererBasicMeshHandler> BasicMeshHandler;
	TVoxelSharedRef<FVoxelRendererClusteredMeshHandler> ClusteredMeshHandler;

	struct FChunkInfo
	{
		const int32 LOD;
		const FIntVector Position;
		
		FChunkId BasicChunkId;
		FChunkId ClusteredChunkId;

		FChunkInfo(int32 LOD, const FIntVector& Position)
			: LOD(LOD)
			, Position(Position)
		{
		}
	};
	TVoxelTypedSparseArray<FChunkId, FChunkInfo> ChunkInfos;
};
