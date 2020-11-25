// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#if ENGINE_MINOR_VERSION < 26
#include "PhysicsEngine/BodySetupEnums.h"
#else
#include "BodySetupEnums.h"
#endif
#include "VoxelEnums.h"
#include "VoxelIntBox.h"
#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelContainers/VoxelStaticArray.h"

struct FVoxelMaterialIndices;
class FInvokerPositionsArray;
class FVoxelPool;
class FVoxelData;
class FVoxelTexturePool;
class FVoxelDebugManager;
class FVoxelToolRenderingManager;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UVoxelMaterialCollectionBase;
class UVoxelProceduralMeshComponent;
class AVoxelWorld;
class AActor;
struct FVoxelChunkUpdate;

DECLARE_MULTICAST_DELEGATE(FVoxelRendererOnWorldLoaded);
// Fired once per chunk
DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelOnChunkUpdateFinished, FVoxelIntBox);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FVoxelOnMaterialInstanceCreated, int32 /*ChunkLOD*/, const FVoxelIntBox& /*ChunkBounds*/, UMaterialInstanceDynamic* /*Instance*/);

struct FVoxelRendererDynamicSettings
{
	struct FLODData
	{
		TWeakObjectPtr<UMaterialInterface> Material;
		TWeakObjectPtr<UVoxelMaterialCollectionBase> MaterialCollection;
		FThreadSafeCounter MaxMaterialIndices = 1;
	};
	
	TVoxelStaticArray<FLODData, 32> LODData{ ForceInit };
};

struct FVoxelRendererMemory
{
	struct FLODStats
	{
		FThreadSafeCounter64 Indices;
		FThreadSafeCounter64 Positions;
		FThreadSafeCounter64 Colors;
		FThreadSafeCounter64 Adjacency;
		FThreadSafeCounter64 UVsAndTangents;
	};
	TVoxelStaticArray<FLODStats, 32> LODs{ ForceInit };

	FThreadSafeCounter64 CollisionMemory;
};

// Doesn't keep alive anything important
struct FVoxelRendererSettingsBase
{
	float VoxelSize;
	TVoxelSharedRef<FIntVector> WorldOffset;

	// Always valid
	UClass* ProcMeshClass;
	bool bCastFarShadow;

	EVoxelPlayType PlayType;

	TWeakObjectPtr<UWorld> World;
	TWeakObjectPtr<UPrimitiveComponent> RootComponent;

	EVoxelUVConfig UVConfig;
	float UVScale;
	EVoxelNormalConfig NormalConfig;
	EVoxelMaterialConfig MaterialConfig;
	bool bHardColorTransitions;
	
	float BoundsExtension;

	ECollisionTraceFlag CollisionTraceFlag;
	int32 NumConvexHullsPerAxis;
	bool bCleanCollisionMeshes;

	EVoxelRenderType RenderType;
	uint32 RenderSharpness;
	bool bCreateMaterialInstances;
	bool bDitherChunks;
	float ChunksDitheringDuration;
	bool bOptimizeIndices;

	int32 MaxDistanceFieldLOD;
	int32 DistanceFieldBoundsExtension;
	int32 DistanceFieldResolutionDivisor;
	float DistanceFieldSelfShadowBias;
	
	bool bOneMaterialPerCubeSide;
	bool bHalfPrecisionCoordinates;
	bool bInterpolateColors;
	bool bInterpolateUVs;
	bool bSRGBColors;
	bool bRenderWorld;
	bool bContributesToStaticLighting;
	bool bUseStaticPath;

	float MeshUpdatesBudget;

	TArray<uint8> HolesMaterials;
	TMap<uint8, FVoxelMeshConfig> MaterialsMeshConfigs;

	bool bMergeChunks;
	int32 ChunksClustersSize;
	bool bDoNotMergeCollisionsAndNavmesh;

	bool bStaticWorld;
	bool bGreedyCubicMesher;
	bool bSimpleCubicCollision;
	int32 SimpleCubicCollisionLODBias;

	TVoxelSharedRef<FVoxelRendererDynamicSettings> DynamicSettings;

	TVoxelSharedRef<FVoxelRendererMemory> Memory = MakeVoxelShared<FVoxelRendererMemory>();

	// If Data isn't null, it's Depth and WorldBounds will be used, and WorldOffset will be set to 0
	FVoxelRendererSettingsBase(
		const AVoxelWorld* World, 
		EVoxelPlayType PlayType,
		UPrimitiveComponent* RootComponent,	
		const FVoxelData* Data = nullptr);

public:
	inline FVector GetChunkRelativePosition(const FIntVector& Position) const
	{
		return FVector(Position + *WorldOffset) * VoxelSize;
	}

	UMaterialInterface* GetVoxelMaterial(int32 LOD, const FVoxelMaterialIndices& MaterialIndices) const;
	UMaterialInterface* GetVoxelMaterial(int32 LOD) const;

	inline void OnMaterialsChanged() const
	{
		// Needed to have errors display again
		UniqueId = UniqueIdCounter++;
	}
	
private:
	mutable uint64 UniqueId = UniqueIdCounter++;
	static uint64 UniqueIdCounter;
};

struct FVoxelRendererSettings : FVoxelRendererSettingsBase
{
	// SharedPtr ones might be null in asset actors
	const TVoxelSharedRef<const FVoxelData> Data;
	const TVoxelSharedRef<FVoxelPool> Pool;
	const TVoxelSharedPtr<FVoxelToolRenderingManager> ToolRenderingManager;
	const TVoxelSharedRef<FVoxelTexturePool> TexturePool;
	const TVoxelSharedRef<FVoxelDebugManager> DebugManager;

	FVoxelRendererSettings(
		const AVoxelWorld* World, 
		EVoxelPlayType PlayType,
		UPrimitiveComponent* RootComponent,
		const TVoxelSharedRef<const FVoxelData>& Data,
		const TVoxelSharedRef<FVoxelPool>& Pool,
		const TVoxelSharedPtr<FVoxelToolRenderingManager>& ToolRenderingManager,
		const TVoxelSharedRef<FVoxelTexturePool>& TexturePool,
		const TVoxelSharedRef<FVoxelDebugManager>& DebugManager,
		bool bUseDataSettings);
};

class VOXEL_API IVoxelRenderer
{
public:
	const FVoxelRendererSettings Settings;
	FVoxelRendererOnWorldLoaded OnWorldLoaded;
	FVoxelOnMaterialInstanceCreated OnMaterialInstanceCreated;

	explicit IVoxelRenderer(const FVoxelRendererSettings& Settings);
	virtual ~IVoxelRenderer() = default;

	//~ Begin IVoxelRenderer Interface
	virtual void Destroy() = 0;

	virtual int32 UpdateChunks(
		const FVoxelIntBox& Bounds,
		const TArray<uint64>& ChunksToUpdate, 
		const FVoxelOnChunkUpdateFinished& FinishDelegate) = 0;
	virtual void UpdateLODs(uint64 InUpdateIndex, const TArray<FVoxelChunkUpdate>& ChunkUpdates) = 0;

	virtual int32 GetTaskCount() const = 0;
	virtual bool AreChunksDithering() const = 0;

	virtual void RecomputeMeshPositions() = 0;
	virtual void ApplyNewMaterials() = 0;
	virtual void ApplyToAllMeshes(TFunctionRef<void(UVoxelProceduralMeshComponent&)> Lambda) = 0;
	
	virtual void CreateGeometry_AnyThread(int32 LOD, const FIntVector& ChunkPosition, TArray<uint32>& OutIndices, TArray<FVector>& OutVertices) const = 0;
	//~ End IVoxelRenderer Interface

	// Called by LOD manager
	void SetInvokersPositionsForPriorities(const TArray<FIntVector>& NewInvokersPositionsForPriorities);
	
	// Used by render chunks to compute the priorities
	inline const TVoxelSharedRef<FInvokerPositionsArray>& GetInvokersPositionsForPriorities() const
	{
		return InvokersPositionsForPriorities;
	}

private:
	TVoxelSharedRef<FInvokerPositionsArray> InvokersPositionsForPriorities;
};