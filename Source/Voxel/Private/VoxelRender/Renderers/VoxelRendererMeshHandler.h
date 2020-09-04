// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelContainers/VoxelSparseArray.h"
#include "VoxelRender/VoxelChunkToUpdate.h"
#include "VoxelRender/IVoxelProceduralMeshComponent_PhysicsCallbackHandler.h"

enum class EDitheringType : uint8;
struct FVoxelChunkSettings;
struct FVoxelChunkMesh;
struct FVoxelIntBox;
class IVoxelRenderer;
class FDistanceFieldVolumeData;
class UVoxelProceduralMeshComponent;
template <class T>
class TAutoConsoleVariable;

#define CHECK_CHUNK_IDS DO_CHECK

extern TAutoConsoleVariable<int32> CVarLogActionQueue;

class IVoxelRendererMeshHandler : public IVoxelProceduralMeshComponent_PhysicsCallbackHandler
{
public:
	DEFINE_TYPED_VOXEL_SPARSE_ARRAY_ID(FChunkId);
	
	IVoxelRenderer& Renderer;

	explicit IVoxelRendererMeshHandler(IVoxelRenderer& Renderer);
	virtual ~IVoxelRendererMeshHandler();
	
	void Init();
	
	FChunkId AddChunk(int32 LOD, const FIntVector& Position);
	void UpdateChunk(
		FChunkId ChunkId,
		const FVoxelChunkSettings& ChunkSettings,
		const FVoxelChunkMesh& MainChunk,
		const FVoxelChunkMesh* TransitionChunk,
		uint8 TransitionsMask); // TransitionsMask isn't always the ChunkSettings one (eg transitions task not done yet), so force specifying it
	void RemoveChunk(FChunkId ChunkId);

	void DitherChunk(FChunkId ChunkId, EDitheringType DitheringType);
	void ResetDithering(FChunkId ChunkId);

	void SetTransitionsMaskForSurfaceNets(FChunkId ChunkId, uint8 TransitionsMask);

	// We don't want to call Update for that as we can keep the collision data
	// Note: doesn't work with clustered mesh handler! Can only be used for dithering
	void HideChunk(FChunkId ChunkId);
	void ShowChunk(FChunkId ChunkId);

	// Used for ApplyNewMaterials
	virtual void ClearChunkMaterials() = 0;

	virtual void Tick(double MaxTime);

public:
	virtual void RecomputeMeshPositions();
	virtual void ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda);
	virtual void StartDestroying();

	inline bool IsDestroying() const
	{
		return bIsDestroying;
	}
	
protected:
	UVoxelProceduralMeshComponent* GetNewMesh(FChunkId ChunkId, const FIntVector& Position, uint8 LOD);
	void RemoveMesh(UVoxelProceduralMeshComponent& Mesh);
	TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& CleanUp(TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>>& Meshes) const;

protected:
	enum class EAction : uint8
	{
		UpdateChunk,
		RemoveChunk,
		DitherChunk,
		ResetDithering,
		SetTransitionsMaskForSurfaceNets,
		HideChunk,
		ShowChunk
	};
	struct FAction
	{
		EAction Action = EAction(-1);
		FChunkId ChunkId;

		FString ToString() const;
		
		struct FUpdateChunk
		{
			struct
			{
				FVoxelChunkSettings ChunkSettings;
				const FVoxelChunkMesh* MainChunk; // Always valid
				const FVoxelChunkMesh* TransitionChunk; // Might be null
			} InitialCall;
			struct
			{
				// This is used to check if the mesh has been updated
				int32 UpdateIndex;
				TVoxelSharedPtr<const FDistanceFieldVolumeData> DistanceFieldVolumeData;
			} AfterCall;
		};
		struct FDitherChunk
		{
			EDitheringType DitheringType;
		};
		struct FSetTransitionsMaskForSurfaceNets
		{
			uint8 TransitionsMask;
		};

#define ACCESSOR(Name) \
	inline F##Name& Name() { check(Action == EAction::Name); return _##Name; } \
	inline const F##Name& Name() const { check(Action == EAction::Name); return _##Name; }
		ACCESSOR(UpdateChunk);
		ACCESSOR(DitherChunk);
		ACCESSOR(SetTransitionsMaskForSurfaceNets);
#undef ACCESSOR
		
	private:
		FUpdateChunk _UpdateChunk{};
		FDitherChunk _DitherChunk{};
		FSetTransitionsMaskForSurfaceNets _SetTransitionsMaskForSurfaceNets{};
	};
	
	virtual FChunkId AddChunkImpl(int32 LOD, const FIntVector& Position) = 0;
	virtual void ApplyAction(const FAction& Action) = 0;

private:
	TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>> MeshPool;
	// Mesh pool containing frozen meshes that can't be used until collisions aren't frozen anymore
	TArray<TWeakObjectPtr<UVoxelProceduralMeshComponent>> FrozenMeshPool;
	// Meshes are only moved once on creation (reusing from pool = creation too)
	// We keep their voxel position here to recompute their position when rebasing
	TMap<TWeakObjectPtr<UVoxelProceduralMeshComponent>, FIntVector> ActiveMeshes;
	// Sanity check
	bool bIsInit = false;
	// Used to skip clearing mesh sections when the renderer is destroying
	bool bIsDestroying = false;

#if CHECK_CHUNK_IDS
	TSet<FChunkId> ValidIndices;
#endif
	
	void SetMeshPosition(UVoxelProceduralMeshComponent& Mesh, const FIntVector& Position) const;
	void OnFreezeVoxelCollisionChanged(bool bNewFreezeCollisions);
};