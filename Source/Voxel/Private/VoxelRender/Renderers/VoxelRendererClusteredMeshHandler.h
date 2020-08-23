// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelQueueWithNum.h"
#include "Containers/StaticArray.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRendererMeshHandler.h"

class FVoxelRendererClusteredMeshHandler : public IVoxelRendererMeshHandler
{
public:
	using IVoxelRendererMeshHandler::IVoxelRendererMeshHandler;
	virtual ~FVoxelRendererClusteredMeshHandler() override;
	
	//~ Begin IVoxelRendererMeshHandler Interface
	virtual FChunkId AddChunkImpl(int32 LOD, const FIntVector& Position) final override;
	virtual void ApplyAction(const FAction& Action) final override;
	virtual void ClearChunkMaterials() final override;
	virtual void Tick(double MaxTime) final override;
	//~ End IVoxelRendererMeshHandler Interface

private:
	DEFINE_TYPED_VOXEL_SPARSE_ARRAY_ID(FClusterId);
	
	struct FClusterBuiltData
	{
		int32 UpdateIndex = -1;
		TUniquePtr<FVoxelBuiltChunkMeshes> BuiltMeshes;
	};
	struct FCluster
	{
		const uint64 UniqueId; // Unique ID used for tasks callbacks
		const int32 LOD;
		const FIntVector Position;
		
		int32 NumChunks = 0;
		
		TSharedPtr<FVoxelChunkMaterials> Materials;
		TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>> Meshes;

		// Used to see if an async build task is still valid
		// Shared ptr: so that the async task can check as well
		TVoxelSharedPtr<FThreadSafeCounter> UpdateIndex;
		// Update index used when last updating the meshes
		int32 MeshUpdateIndex = -1;
		
		// Processed data waiting to be displayed
		FClusterBuiltData BuiltData;

		// Chunk unique id -> its meshes
		// Shared ptr: used by build task
		TMap<uint64, TVoxelSharedPtr<const FVoxelChunkMeshesToBuild>> ChunkMeshesToBuild;

		static FCluster Create(
			int32 LOD,
			const FIntVector& Position)
		{
			return FCluster(UNIQUE_ID(), LOD, Position);
		}

	private:
		explicit FCluster(
			uint64 UniqueId,
			int32 LOD,
			const FIntVector& Position)
			: UniqueId(UniqueId)
			, LOD(LOD)
			, Position(Position)
		{
		}
	};
	TVoxelTypedSparseArray<FClusterId, FCluster> Clusters;

	struct FClusterRef
	{
		FClusterId ClusterId;
		uint64 UniqueId = 0;
	};
	inline FCluster* GetCluster(FClusterRef Ref)
	{
		if (!Clusters.IsValidIndex(Ref.ClusterId))
		{
			return nullptr;
		}
		auto& Cluster = Clusters[Ref.ClusterId];
		if (Cluster.UniqueId != Ref.UniqueId)
		{
			return nullptr;
		}
		return &Cluster;
	}
	
	TStaticArray<TMap<FIntVector, FClusterRef>, 32> ClustersMap; // Max 32 LODs

	FClusterRef FindOrCreateCluster(int32 LOD, const FIntVector& Position);

	struct FChunkInfo
	{
		const uint64 UniqueId; // Unique ID used in cluster map
		
		const int32 LOD;
		const FIntVector Position;

		FClusterId ClusterId;

		static FChunkInfo Create(
			int32 LOD,
			const FIntVector& Position)
		{
			return FChunkInfo(UNIQUE_ID(), LOD, Position);
		}

	private:
		FChunkInfo(
			uint64 UniqueId,
			int32 LOD,
			const FIntVector& Position)
			: UniqueId(UniqueId)
			, LOD(LOD)
			, Position(Position)
		{
		}
	};
	TVoxelTypedSparseArray<FChunkId, FChunkInfo> ChunkInfos;

	struct FBuildCallback
	{
		FClusterRef ClusterRef;
		FClusterBuiltData BuiltData;
	};
	TQueue<FBuildCallback, EQueueMode::Mpsc> CallbackQueue;

	// Queue all actions
	// This ensures they are processed sequentially even when using async tasks
	TVoxelQueueWithNum<FAction, EQueueMode::Spsc> ActionQueue;

	void FlushBuiltDataQueue();
	void FlushActionQueue(double MaxTime);
	void MeshMergeCallback(FClusterRef ClusterRef, int32 UpdateIndex, TUniquePtr<FVoxelBuiltChunkMeshes> BuiltMeshes);

	friend class FVoxelClusteredMeshMergeWork;
};