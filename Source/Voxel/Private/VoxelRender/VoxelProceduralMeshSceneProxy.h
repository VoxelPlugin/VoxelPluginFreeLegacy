// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "StaticMeshResources.h"
#include "PrimitiveSceneProxy.h"
#include "VoxelMinimal.h"
#if RHI_RAYTRACING
#include "RayTracingDefinitions.h"
#include "RayTracingInstance.h"
#endif

class FVoxelToolRenderingManager;
class UVoxelProceduralMeshComponent;
class FVoxelMaterialInterface;
struct FVoxelProcMeshBuffers;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Mesh Distance Field Memory"), STAT_VoxelMeshDistanceFieldMemory, STATGROUP_VoxelMemory, VOXEL_API);

class FVoxelProcMeshBuffersRenderData : public TVoxelSharedFromThis<FVoxelProcMeshBuffersRenderData>
{
public:
	const TVoxelSharedRef<const FVoxelProcMeshBuffers> Buffers;
	
	FLocalVertexFactory VertexFactory;
#if RHI_RAYTRACING
	FRayTracingGeometry RayTracingGeometry;
#endif

	static TVoxelSharedRef<FVoxelProcMeshBuffersRenderData> GetRenderData(
		const TVoxelSharedRef<const FVoxelProcMeshBuffers>& Buffers,
		ERHIFeatureLevel::Type FeatureLevel);
	~FVoxelProcMeshBuffersRenderData();

private:
	explicit FVoxelProcMeshBuffersRenderData(
		const TVoxelSharedRef<const FVoxelProcMeshBuffers>& Buffers,
		ERHIFeatureLevel::Type FeatureLevel);
};

struct FVoxelProcMeshProxySection
{
	TVoxelSharedPtr<FVoxelMaterialInterface> Material;
	TVoxelSharedPtr<const FVoxelProcMeshBuffers> Buffers;
	TVoxelSharedPtr<FVoxelProcMeshBuffersRenderData> RenderData;
	
	bool bSectionVisible = true;
	bool bRequiresAdjacencyInformation = false;

	bool bEnableCollisions_Debug = false;
	bool bEnableNavmesh_Debug = false;
};

class FVoxelProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	explicit FVoxelProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component);
	~FVoxelProceduralMeshSceneProxy();

	//~ Begin FPrimitiveSceneProxy Interface
	virtual void CreateRenderThreadResources() override;
	ONLY_UE_24_AND_HIGHER(virtual) void DestroyRenderThreadResources() ONLY_UE_24_AND_HIGHER(override);
	
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

#if RHI_RAYTRACING
	virtual bool IsRayTracingRelevant() const override { return true; }
	virtual void GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances) override;
#endif
	
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual bool CanBeOccluded() const override;
	virtual uint32 GetMemoryFootprint() const override;
	virtual SIZE_T GetTypeHash() const override;
	uint32 GetAllocatedSize() const;
	
	virtual void GetDistancefieldAtlasData(
		FBox& LocalVolumeBounds,
		FVector2D& OutDistanceMinMax,
		FIntVector& OutBlockMin,
		FIntVector& OutBlockSize,
		bool& bOutBuiltAsIfTwoSided,
		bool& bMeshWasPlane,
		float& SelfShadowBias,
		TArray<FMatrix>& ObjectLocalToWorldTransforms,
		bool& bOutThrottled) const override;
	virtual void GetDistanceFieldInstanceInfo(int32& NumInstances, float& BoundsSurfaceArea) const override;
	virtual bool HasDistanceFieldRepresentation() const override;
	virtual bool HasDynamicIndirectShadowCasterRepresentation() const override;
	//~ End FPrimitiveSceneProxy Interface

private:
	UVoxelProceduralMeshComponent* const Component;
	const FMaterialRelevance MaterialRelevance;
	const int32 LOD;
	const uint32 DebugChunkId;
	const TVoxelWeakPtr<const FVoxelToolRenderingManager> WeakToolRenderingManager;
	
	const FCollisionResponseContainer CollisionResponse;
	const ECollisionTraceFlag CollisionTraceFlag;

	TArray<FVoxelProcMeshProxySection> Sections;
	TVoxelSharedPtr<const FDistanceFieldVolumeData> DistanceFieldData;

	double FinishSectionsUpdatesTime = 0;
	double CreateSceneProxyTime = 0;
	mutable bool bLoggedTime = false;

	FMeshBatch& DrawSection(
		FMeshElementCollector& Collector,
		const FVoxelProcMeshProxySection& Section, 
		const FMaterialRenderProxy* MaterialRenderProxy,
		bool bEnableTessellation,
		bool bWireframe) const;
	
	bool ShouldDrawComplexCollisions(const FEngineShowFlags& EngineShowFlags) const;
};