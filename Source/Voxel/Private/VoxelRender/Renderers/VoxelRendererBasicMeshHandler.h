// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelQueueWithNum.h"
#include "VoxelMinimal.h"
#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRendererMeshHandler.h"

class FVoxelRendererBasicMeshHandler : public IVoxelRendererMeshHandler
{
public:
	using IVoxelRendererMeshHandler::IVoxelRendererMeshHandler;
	virtual ~FVoxelRendererBasicMeshHandler() override;
	
	//~ Begin IVoxelRendererMeshHandler Interface
	virtual FChunkId AddChunkImpl(int32 LOD, const FIntVector& Position) final override;
	virtual void ApplyAction(const FAction& Action) final override;
	virtual void ClearChunkMaterials() final override;
	virtual void Tick(double MaxTime) final override;
	//~ End IVoxelRendererMeshHandler Interface
	
private:
	struct FChunkBuiltData
	{
		int32 UpdateIndex = -1;
		TUniquePtr<FVoxelBuiltChunkMeshes> BuiltMeshes;
	};
	struct FChunkInfo
	{
		const uint64 UniqueId; // Unique ID used for tasks callbacks. Can't use ChunkId as it might get reused
		const int32 LOD;
		const FIntVector Position;
		
		TSharedPtr<FVoxelChunkMaterials> Materials;
		TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>> Meshes;
		
		// Used to see if an async build task is still valid
		// Shared ptr: so that the async task can check as well
		TVoxelSharedPtr<FThreadSafeCounter> UpdateIndex;
		// Update index used when last updating the meshes
		int32 MeshUpdateIndex = -1;
		// Processed data waiting to be displayed
		FChunkBuiltData BuiltData;

		// Record last dithering state to be applied on new materials
		// Needed as we can't reliably read that state from the materials
		FVoxelRenderUtilities::FDitheringInfo DitheringInfo;

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

	struct FChunkInfoRef
	{
		FChunkId ChunkId;
		uint64 UniqueId = 0;
	};
	inline FChunkInfo* GetChunkInfo(FChunkInfoRef Ref)
	{
		if (!ChunkInfos.IsValidIndex(Ref.ChunkId))
		{
			return nullptr;
		}
		auto& ChunkInfo = ChunkInfos[Ref.ChunkId];
		if (ChunkInfo.UniqueId != Ref.UniqueId)
		{
			return nullptr;
		}
		return &ChunkInfo;
	}

	struct FBuildCallback
	{
		FChunkInfoRef ChunkInfoRef;
		FChunkBuiltData BuiltData;
	};
	TQueue<FBuildCallback, EQueueMode::Mpsc> CallbackQueue;

	// Queue all actions
	// This ensures they are processed sequentially even when using async tasks
	TVoxelQueueWithNum<FAction, EQueueMode::Spsc> ActionQueue;

	void FlushBuiltDataQueue();
	void FlushActionQueue(double MaxTime);
	void MeshMergeCallback(FChunkInfoRef ChunkInfoRef, int32 UpdateIndex, TUniquePtr<FVoxelBuiltChunkMeshes> BuiltMeshes);

	friend class FVoxelBasicMeshMergeWork;
};