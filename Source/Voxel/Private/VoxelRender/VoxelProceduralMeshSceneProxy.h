// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "StaticMeshResources.h"
#include "VoxelRawStaticIndexBuffer.h"
#include "PrimitiveSceneProxy.h"

class UMaterialInterface;
class UVoxelProceduralMeshComponent;

// TODO: memory stats

class FVoxelProcMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material = nullptr;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FVoxelRawStaticIndexBuffer IndexBuffer;
	/** Index buffer containing adjacency information required by tessellation. */
	FVoxelRawStaticIndexBuffer AdjacencyIndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible = true;
	/** For tessellation */
	bool bRequiresAdjacencyInformation = false;

	FVoxelProcMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: VertexFactory(InFeatureLevel, "FVoxelProcMeshProxySection")
	{
	}

	void InitResources();
	void ReleaseResources();
};

class FVoxelProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	FVoxelProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component);
	~FVoxelProceduralMeshSceneProxy();

	void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	bool CanBeOccluded() const override;
	uint32 GetMemoryFootprint() const override;
	uint32 GetAllocatedSize() const;

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

private:
	TArray<FVoxelProcMeshProxySection*> Sections;
	UBodySetup* const BodySetup;
	FMaterialRelevance const MaterialRelevance;
};