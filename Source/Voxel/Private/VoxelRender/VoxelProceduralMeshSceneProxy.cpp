// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelProceduralMeshSceneProxy.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelGlobals.h"

#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "TessellationRendering.h"
#include "PhysicsEngine/BodySetup.h"
#include "DistanceFieldAtlas.h"

#define NOT_SHIPPING_NOR_TEST !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

static TAutoConsoleVariable<int32> CVarLogProcMeshDelays(
	TEXT("voxel.renderer.LogProcMeshDelays"),
	0,
	TEXT("If true, will log the time elapsed between the game thread update and the first render thread display"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowToolRendering(
	TEXT("voxel.renderer.ShowToolRendering"),
	0,
	TEXT("If true, will show the duplicated meshes for tool rendering in red"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowMeshSections(
	TEXT("voxel.renderer.ShowMeshSections"),
	0,
	TEXT("If true, will assign a unique color to each mesh section"),
	ECVF_Default);

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelMeshDistanceFieldMemory);

DECLARE_DWORD_COUNTER_STAT(TEXT("Num Voxel Draw Calls"), STAT_NumVoxelDrawCalls, STATGROUP_VoxelCounters);
DECLARE_DWORD_COUNTER_STAT(TEXT("Num Voxel Draw Calls For Tools"), STAT_NumVoxelDrawCallsForTools, STATGROUP_VoxelCounters);

DECLARE_DWORD_COUNTER_STAT(TEXT("Num Voxel Triangles Drawn "), STAT_NumVoxelTrianglesDrawn, STATGROUP_VoxelCounters);
DECLARE_DWORD_COUNTER_STAT(TEXT("Num Voxel Triangles Drawn  For Tools"), STAT_NumVoxelTrianglesDrawnForTools, STATGROUP_VoxelCounters);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelProcMeshBuffersRenderData::FVoxelProcMeshBuffersRenderData(
	const TVoxelSharedRef<const FVoxelProcMeshBuffers>& Buffers,
	ERHIFeatureLevel::Type FeatureLevel)
	: Buffers(Buffers)
	, VertexFactory(FeatureLevel, "FVoxelProcMeshBuffersRenderData")
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	{
		auto& InitBuffers = const_cast<FVoxelProcMeshBuffers&>(*Buffers);
		BeginInitResource(&InitBuffers.VertexBuffers.PositionVertexBuffer);
		BeginInitResource(&InitBuffers.VertexBuffers.StaticMeshVertexBuffer);
		BeginInitResource(&InitBuffers.VertexBuffers.ColorVertexBuffer);
		BeginInitResource(&InitBuffers.IndexBuffer);
		BeginInitResource(&InitBuffers.AdjacencyIndexBuffer);
	}

	auto& VertexBuffers = Buffers->VertexBuffers;
	auto& IndexBuffer = Buffers->IndexBuffer;
	
	FLocalVertexFactory::FDataType Data;
	VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, Data);
	VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, Data);
	VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&VertexFactory, Data);
	VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&VertexFactory, Data);
	VertexFactory.SetData(Data);
	VertexFactory.InitResource();

#if RHI_RAYTRACING
	if (IsRayTracingEnabled())
	{
		FRayTracingGeometryInitializer Initializer;
		Initializer.IndexBuffer = IndexBuffer.IndexBufferRHI;
		Initializer.TotalPrimitiveCount = IndexBuffer.GetNumIndices() / 3;
#if ENGINE_MINOR_VERSION < 23
		Initializer.PrimitiveType = PT_TriangleList;
#else
		Initializer.GeometryType = RTGT_Triangles;
#endif
		Initializer.bFastBuild = true;
		Initializer.bAllowUpdate = false;

#if ENGINE_MINOR_VERSION < 24
		Initializer.PositionVertexBuffer = VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
		Initializer.BaseVertexIndex = 0;
		Initializer.VertexBufferStride = VertexBuffers.PositionVertexBuffer.GetStride();
		Initializer.VertexBufferByteOffset = 0;
		Initializer.VertexBufferElementType = VET_Float3;
#else
		FRayTracingGeometrySegment Segment;
		Segment.VertexBuffer = VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
		Segment.NumPrimitives = Initializer.TotalPrimitiveCount;
		Initializer.Segments.Add(Segment);
#endif

		RayTracingGeometry.SetInitializer(Initializer);
		RayTracingGeometry.InitResource();
	}
#endif
}

TVoxelSharedRef<FVoxelProcMeshBuffersRenderData> FVoxelProcMeshBuffersRenderData::GetRenderData(
	const TVoxelSharedRef<const FVoxelProcMeshBuffers>& Buffers,
	ERHIFeatureLevel::Type FeatureLevel)
{
	check(IsInRenderingThread());
	if (!Buffers->RenderData.IsValid())
	{
		auto Result = TVoxelSharedRef<FVoxelProcMeshBuffersRenderData>(new FVoxelProcMeshBuffersRenderData(Buffers, FeatureLevel));
		Buffers->RenderData = Result;
		return Result;
	}
	else
	{
		return Buffers->RenderData.Pin().ToSharedRef();
	}
}
FVoxelProcMeshBuffersRenderData::~FVoxelProcMeshBuffersRenderData()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());

	auto& InitBuffers = const_cast<FVoxelProcMeshBuffers&>(*Buffers);
	InitBuffers.VertexBuffers.PositionVertexBuffer.ReleaseResource();
	InitBuffers.VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	InitBuffers.VertexBuffers.ColorVertexBuffer.ReleaseResource();
	InitBuffers.IndexBuffer.ReleaseResource();
	InitBuffers.AdjacencyIndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
		
#if RHI_RAYTRACING
	if (IsRayTracingEnabled())
	{
		RayTracingGeometry.ReleaseResource();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline bool IsCollisionView(const FEngineShowFlags& EngineShowFlags)
{
	return EngineShowFlags.CollisionVisibility || EngineShowFlags.CollisionPawn;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelProceduralMeshSceneProxy::FVoxelProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component)
	: FPrimitiveSceneProxy(Component)
	, Component(Component)
	, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	, LOD(Component->LOD)
	, DebugChunkId(Component->DebugChunkId)
	, FrameToClearPreviousLocalToWorld(Component->bNewInit ? GFrameNumber : 0)
	, WeakToolRenderingManager(Component->ToolRenderingManager)
	, CollisionResponse(Component->GetCollisionResponseToChannels())
	, CollisionTraceFlag(Component->CollisionTraceFlag)
{
	VOXEL_FUNCTION_COUNTER();
	
	// Proxy settings
	bSupportsDistanceFieldRepresentation = true;
	bVerifyUsedMaterials = false; // Fails with tool rendering

	Component->bNewInit = false;

	FinishSectionsUpdatesTime = Component->LastFinishSectionsUpdatesTime;
	CreateSceneProxyTime = FPlatformTime::Seconds();

	// Copy distance field data
	DistanceFieldData = Component->DistanceFieldData;

	// Copy each section
	const int32 NumSections = Component->ProcMeshSections.Num();
	Sections.SetNum(NumSections);
	for (int32 SectionIndex = 0; SectionIndex < NumSections; SectionIndex++)
	{
		const auto& SrcSection = Component->ProcMeshSections[SectionIndex];
		FVoxelProcMeshProxySection& NewSection = Sections[SectionIndex];

		ensure(SrcSection.Settings.bSectionVisible || SrcSection.Settings.bEnableCollisions || SrcSection.Settings.bEnableNavmesh);

		if (SrcSection.Buffers->GetNumVertices() == 0)
		{
			NewSection.bSectionVisible = false;
			continue;
		}

		// Visibility
		NewSection.bSectionVisible = SrcSection.Settings.bSectionVisible;

		// Copy debug info
		NewSection.bEnableCollisions_Debug = SrcSection.Settings.bEnableCollisions;
		NewSection.bEnableNavmesh_Debug = SrcSection.Settings.bEnableNavmesh;

		// Grab material
		NewSection.Material = SrcSection.Settings.Material;
		if (!NewSection.Material.IsValid())
		{
			NewSection.Material = FVoxelMaterialInterfaceManager::Get().DefaultMaterial();
		}
		check(NewSection.Material.IsValid());

		// Copy buffer ptr
		NewSection.Buffers = SrcSection.Buffers;

		// Tessellation
		auto& IndexBuffer = NewSection.Buffers->IndexBuffer;
		auto& AdjacencyIndexBuffer = NewSection.Buffers->AdjacencyIndexBuffer;

		check(
			AdjacencyIndexBuffer.GetNumIndices() == 0 ||
			AdjacencyIndexBuffer.GetNumIndices() == 4 * IndexBuffer.GetNumIndices());

		const bool bTessellatedMaterial =
			RequiresAdjacencyInformation(
				NewSection.Material->GetMaterial(),
				&FLocalVertexFactory::StaticType,
				GetScene().GetFeatureLevel());
		const bool bHasAdjacency = AdjacencyIndexBuffer.GetNumIndices() > 0;

		ensure(SrcSection.Settings.bEnableTessellation == bHasAdjacency);
		ensureMsgf(bTessellatedMaterial == bHasAdjacency, TEXT("Invalid tessellated material or non tessellated material is tessellated"));
		NewSection.bRequiresAdjacencyInformation = bTessellatedMaterial && bHasAdjacency;
	}
}

FVoxelProceduralMeshSceneProxy::~FVoxelProceduralMeshSceneProxy()
{
	ONLY_UE_23_AND_LOWER(DestroyRenderThreadResources());
	for (auto& Section : Sections)
	{
		check(!Section.RenderData.IsValid());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelProceduralMeshSceneProxy::CreateRenderThreadResources()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());
	
	for (auto& Section : Sections)
	{
		check(!Section.RenderData.IsValid());
		check(Section.Buffers.IsValid());
		if (Section.bSectionVisible || NOT_SHIPPING_NOR_TEST) // Need to init for debug
		{
			Section.RenderData = FVoxelProcMeshBuffersRenderData::GetRenderData(Section.Buffers.ToSharedRef(), GetScene().GetFeatureLevel());
		}
	}
#if ENABLE_VOXEL_DISTANCE_FIELDS
	if (DistanceFieldData.IsValid())
	{
		const_cast<FDistanceFieldVolumeTexture&>(DistanceFieldData->VolumeTexture).Initialize(reinterpret_cast<UStaticMesh*>(Component)); // Horrible hack, but w/e if it works :)
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelMeshDistanceFieldMemory, DistanceFieldData->GetResourceSizeBytes());
	}
#endif
}

void FVoxelProceduralMeshSceneProxy::DestroyRenderThreadResources()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInRenderingThread());
	
	for (auto& Section : Sections)
	{
		Section.RenderData.Reset();
	}
	
#if ENABLE_VOXEL_DISTANCE_FIELDS
	if (DistanceFieldData.IsValid())
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelMeshDistanceFieldMemory, DistanceFieldData->GetResourceSizeBytes());
		const_cast<FDistanceFieldVolumeTexture&>(DistanceFieldData->VolumeTexture).Release();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelProceduralMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	VOXEL_FUNCTION_COUNTER();

	const auto& EngineShowFlags = ViewFamily.EngineShowFlags;

	// Hack to see the delay between update call and actual mesh render update
	if (!bLoggedTime && CVarLogProcMeshDelays.GetValueOnRenderThread() != 0 && FinishSectionsUpdatesTime > 0)
	{
		const double Time = FPlatformTime::Seconds();
		LOG_VOXEL(Log, TEXT("Proc Mesh Delays: ChunkId: %u; CreateSceneProxy: %fms; GetDynamicMeshElements: %fms"),
			DebugChunkId,
			(CreateSceneProxyTime - FinishSectionsUpdatesTime) * 1000,
			(Time - FinishSectionsUpdatesTime) * 1000);
		bLoggedTime = true;
	}

#if NOT_SHIPPING_NOR_TEST
	// Render bounds
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			RenderBounds(Collector.GetPDI(ViewIndex), EngineShowFlags, GetBounds(), IsSelected());
		}
	}
	if (IsCollisionView(EngineShowFlags))
	{
		if (ShouldDrawComplexCollisions(EngineShowFlags))
		{
			for (auto& Section : Sections)
			{
				if (!Section.bEnableCollisions_Debug) continue;
				
				const FColor ComplexCollisionColor = FColor(0, 255, 255, 255);
				auto* MaterialProxy = new FColoredMaterialRenderProxy(GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(), ComplexCollisionColor);
				Collector.RegisterOneFrameMaterialProxy(MaterialProxy);

				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (!(VisibilityMap & (1 << ViewIndex))) continue;

					FMeshBatch& Mesh = DrawSection(Collector, Section, MaterialProxy, false, EngineShowFlags.Wireframe);
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}
	else if (FVoxelDebugManager::ShowCollisionAndNavmeshDebug())
	{
		for (auto& Section : Sections)
		{
			if (!Section.RenderData.IsValid()) continue;
			
			const auto* ParentMaterial =
				EngineShowFlags.Wireframe
				? GEngine->WireframeMaterial
				: GEngine->LevelColorationLitMaterial;

			const auto Color = FVoxelDebugManager::GetCollisionAndNavmeshDebugColor(Section.bEnableCollisions_Debug, Section.bEnableNavmesh_Debug);

			if (!ensure(ParentMaterial)) return; // Happens in packaged games
			
			auto* MaterialProxy = new FColoredMaterialRenderProxy(ParentMaterial->GetRenderProxy(), Color);
			Collector.RegisterOneFrameMaterialProxy(MaterialProxy);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (!(VisibilityMap & (1 << ViewIndex))) continue;

				FMeshBatch& Mesh = DrawSection(Collector, Section, MaterialProxy, false, EngineShowFlags.Wireframe);
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}
	else
#endif
	{
		const bool bForceDisableTessellation = !(EngineShowFlags.Materials || EngineShowFlags.Wireframe); // else crash in eg lighting
		
		for (const auto& Section : Sections)
		{
			if (!Section.bSectionVisible || !ensure(Section.Material->GetMaterial()->IsValidLowLevel())) continue;

			auto* MaterialProxy = Section.Material->GetMaterial()->GetRenderProxy();

			if (CVarShowMeshSections.GetValueOnRenderThread() != 0)
			{
				uint32 Hash = 0;
				for (auto& Guid : Section.Buffers->Guids)
				{
					Hash = FVoxelUtilities::MurmurHash64((uint64(Hash) << 32) ^ Guid.A ^ Guid.B ^ Guid.C ^ Guid.D);
				}
				MaterialProxy = new FColoredMaterialRenderProxy(GEngine->LevelColorationLitMaterial->GetRenderProxy(), reinterpret_cast<FColor&>(Hash));
				Collector.RegisterOneFrameMaterialProxy(MaterialProxy);
			}
			
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (!(VisibilityMap & (1 << ViewIndex))) continue;

				FMeshBatch& Mesh = DrawSection(Collector, Section, MaterialProxy, !bForceDisableTessellation, EngineShowFlags.Wireframe);
				Collector.AddMesh(ViewIndex, Mesh);

				INC_DWORD_STAT(STAT_NumVoxelDrawCalls);
				INC_DWORD_STAT_BY(STAT_NumVoxelTrianglesDrawn, Section.Buffers->GetNumIndices() / 3);
			}
		}

		const auto ToolRenderingManager = WeakToolRenderingManager.Pin();
		if (ToolRenderingManager.IsValid())
		{
			VOXEL_SCOPE_COUNTER("Render Tools");

			TArray<FVoxelToolRendering, TInlineAllocator<64>> Tools;

			const FBox WorldBounds = GetBounds().GetBox();
			ToolRenderingManager->IterateTools(
				[&](const FVoxelToolRendering& Tool)
				{
					if (Tool.bEnabled && Tool.WorldBounds.Intersect(WorldBounds))
					{
						Tools.Add(Tool);
					}
				});

			for (auto& Tool : Tools)
			{
				const auto Material =
					Tool.Material.IsValid() && ensure(Tool.Material->GetMaterial()->IsValidLowLevel())
					? Tool.Material
					: FVoxelMaterialInterfaceManager::Get().DefaultMaterial();
				auto* MaterialProxy = Material->GetMaterial()->GetRenderProxy();

				// Hack to fix translucent rendering when the tool material was changed but the mesh wasn't updated
				const_cast<FMaterialRelevance&>(MaterialRelevance) |= Material->GetMaterial()->GetRelevance_Concurrent(GetScene().GetFeatureLevel());

				if (CVarShowToolRendering.GetValueOnRenderThread() != 0)
				{
					MaterialProxy = new FColoredMaterialRenderProxy(GEngine->LevelColorationUnlitMaterial->GetRenderProxy(), FColor::Red);
					Collector.RegisterOneFrameMaterialProxy(MaterialProxy);
				}
				
				for (const auto& Section : Sections)
				{
					if (!Section.bSectionVisible) continue;
					for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
					{
						if (!(VisibilityMap & (1 << ViewIndex))) continue;

						FMeshBatch& Mesh = DrawSection(Collector, Section, MaterialProxy, !bForceDisableTessellation, EngineShowFlags.Wireframe);
						Collector.AddMesh(ViewIndex, Mesh);

						INC_DWORD_STAT(STAT_NumVoxelDrawCallsForTools);
						INC_DWORD_STAT_BY(STAT_NumVoxelTrianglesDrawnForTools, Section.Buffers->GetNumIndices() / 3)
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if RHI_RAYTRACING
void FVoxelProceduralMeshSceneProxy::GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances)
{
	VOXEL_FUNCTION_COUNTER();
	
	for (const auto& Section : Sections)
	{
		if (Section.bSectionVisible && ensure(Section.Material->GetMaterial()->IsValidLowLevel()))
		{
			auto& RenderData = *Section.RenderData;
			if (RenderData.RayTracingGeometry.RayTracingGeometryRHI.IsValid())
			{
				check(RenderData.RayTracingGeometry.Initializer.IndexBuffer.IsValid());

				FRayTracingInstance RayTracingInstance;
				RayTracingInstance.Geometry = &RenderData.RayTracingGeometry;
				RayTracingInstance.InstanceTransforms.Add(GetLocalToWorld());

				FMeshBatch MeshBatch;

				MeshBatch.VertexFactory = &RenderData.VertexFactory;
				MeshBatch.SegmentIndex = 0;
				MeshBatch.MaterialRenderProxy = Section.Material->GetMaterial()->GetRenderProxy();
				MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
				MeshBatch.Type = PT_TriangleList;
				MeshBatch.DepthPriorityGroup = SDPG_World;
				MeshBatch.bCanApplyViewModeOverrides = false;

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Context.RayTracingMeshResourceCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				InitDynamicPrimitiveUniformBuffer(DynamicPrimitiveUniformBuffer);

				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				BatchElement.IndexBuffer = &Section.Buffers->IndexBuffer;
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = Section.Buffers->IndexBuffer.GetNumIndices() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = Section.Buffers->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

				RayTracingInstance.Materials.Add(MeshBatch);

				RayTracingInstance.BuildInstanceMaskAndFlags();
				OutRayTracingInstances.Add(RayTracingInstance);
			}
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPrimitiveViewRelevance FVoxelProceduralMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = IsMovable() && Result.UE_25_SWITCH(bOpaqueRelevance, bOpaque) && Result.bRenderInMainPass;
	return Result;
}

bool FVoxelProceduralMeshSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FVoxelProceduralMeshSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

SIZE_T FVoxelProceduralMeshSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

uint32 FVoxelProceduralMeshSceneProxy::GetAllocatedSize() const
{
	return Sections.GetAllocatedSize() + FPrimitiveSceneProxy::GetAllocatedSize();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if ENABLE_VOXEL_DISTANCE_FIELDS
void FVoxelProceduralMeshSceneProxy::GetDistancefieldAtlasData(
	FBox& LocalVolumeBounds, 
	FVector2D& OutDistanceMinMax, 
	FIntVector& OutBlockMin, 
	FIntVector& OutBlockSize, 
	bool& bOutBuiltAsIfTwoSided, 
	bool& bMeshWasPlane, 
	float& SelfShadowBias,
	TArray<FMatrix>& ObjectLocalToWorldTransforms
#if ENGINE_MINOR_VERSION >= 23
	, bool& bOutThrottled
#endif
) const
{
	if (DistanceFieldData.IsValid())
	{
		LocalVolumeBounds = DistanceFieldData->LocalBoundingBox;
		OutDistanceMinMax = DistanceFieldData->DistanceMinMax;
		OutBlockMin = DistanceFieldData->VolumeTexture.GetAllocationMin();
		OutBlockSize = DistanceFieldData->VolumeTexture.GetAllocationSize();
		bOutBuiltAsIfTwoSided = DistanceFieldData->bBuiltAsIfTwoSided;
		bMeshWasPlane = DistanceFieldData->bMeshWasPlane;
		ObjectLocalToWorldTransforms.Add(GetLocalToWorld());
		SelfShadowBias = DistanceFieldSelfShadowBias;
#if ENGINE_MINOR_VERSION >= 23
		bOutThrottled = DistanceFieldData->VolumeTexture.Throttled();
#endif
	}
	else
	{
		LocalVolumeBounds = FBox(ForceInit);
		OutDistanceMinMax = FVector2D(0, 0);
		OutBlockMin = FIntVector(-1, -1, -1);
		OutBlockSize = FIntVector(0, 0, 0);
		bOutBuiltAsIfTwoSided = false;
		bMeshWasPlane = false;
		SelfShadowBias = 0;
#if ENGINE_MINOR_VERSION >= 23
		bOutThrottled = false;
#endif
	}
}

void FVoxelProceduralMeshSceneProxy::GetDistanceFieldInstanceInfo(int32& NumInstances, float& BoundsSurfaceArea) const
{
	NumInstances = DistanceFieldData.IsValid() ? 1 : 0;
	const FVector AxisScales = GetLocalToWorld().GetScaleVector();
	const FVector BoxDimensions = GetBounds().BoxExtent * AxisScales * 2;

	BoundsSurfaceArea =
		2 * BoxDimensions.X * BoxDimensions.Y +
		2 * BoxDimensions.Z * BoxDimensions.Y +
		2 * BoxDimensions.X * BoxDimensions.Z;
}

bool FVoxelProceduralMeshSceneProxy::HasDistanceFieldRepresentation() const
{
	return CastsDynamicShadow() && AffectsDistanceFieldLighting() && DistanceFieldData.IsValid() && DistanceFieldData->VolumeTexture.IsValidDistanceFieldVolume();
}

bool FVoxelProceduralMeshSceneProxy::HasDynamicIndirectShadowCasterRepresentation() const
{
	return bCastsDynamicIndirectShadow && FVoxelProceduralMeshSceneProxy::HasDistanceFieldRepresentation();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FMeshBatch& FVoxelProceduralMeshSceneProxy::DrawSection(
	FMeshElementCollector& Collector,
	const FVoxelProcMeshProxySection& Section,
	const FMaterialRenderProxy* MaterialRenderProxy,
	bool bEnableTessellation,
	bool bWireframe) const
{
	check(MaterialRenderProxy);
	check(Section.RenderData.IsValid());

	FMeshBatch& Mesh = Collector.AllocateMesh();
	
	Mesh.VertexFactory = &Section.RenderData->VertexFactory;
	Mesh.MaterialRenderProxy = MaterialRenderProxy;
	Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
	Mesh.Type = PT_TriangleList;
	Mesh.DepthPriorityGroup = SDPG_World;
	Mesh.bUseWireframeSelectionColoring = IsSelected() && bWireframe; // Else mesh LODs view is messed up when actor is selected
	Mesh.bCanApplyViewModeOverrides = true;
#if NOT_SHIPPING_NOR_TEST
	Mesh.VisualizeLODIndex = LOD % GEngine->LODColorationColors.Num();
#endif
	
	FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
	InitDynamicPrimitiveUniformBuffer(DynamicPrimitiveUniformBuffer);

	FMeshBatchElement& BatchElement = Mesh.Elements[0];
	BatchElement.IndexBuffer = &Section.Buffers->IndexBuffer;
	BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = Section.Buffers->IndexBuffer.GetNumIndices() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = Section.Buffers->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

#if ENABLE_TESSELLATION
	if (bEnableTessellation)
	{
		// Could be different from bRequiresAdjacencyInformation during shader compilation
		const bool bCurrentRequiresAdjacencyInformation = RequiresAdjacencyInformation(
			MaterialRenderProxy->GetMaterialInterface(),
			&FLocalVertexFactory::StaticType,
			GetScene().GetFeatureLevel());

		if (ensure(Section.Buffers->IndexBuffer.GetNumIndices() != 0) &&
			Section.bRequiresAdjacencyInformation &&
			bCurrentRequiresAdjacencyInformation)
		{
			Mesh.Type = PT_12_ControlPointPatchList;
			BatchElement.IndexBuffer = &Section.Buffers->AdjacencyIndexBuffer;
			BatchElement.FirstIndex *= 4;
		}
	}
#endif

	return Mesh;
}

void FVoxelProceduralMeshSceneProxy::InitDynamicPrimitiveUniformBuffer(FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer) const
{
	bool bHasPrecomputedVolumetricLightmap;
	FMatrix PreviousLocalToWorld;
	int32 SingleCaptureIndex;
#if ENGINE_MINOR_VERSION >= 23
	bool bOutputVelocity;
#endif
	GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
		GetPrimitiveSceneInfo(),
		bHasPrecomputedVolumetricLightmap,
		PreviousLocalToWorld,
		SingleCaptureIndex
#if ENGINE_MINOR_VERSION >= 23
		, bOutputVelocity
#endif
	);

	if (GFrameNumber == FrameToClearPreviousLocalToWorld)
	{
		// Cancel motion blur
		PreviousLocalToWorld = GetLocalToWorld();
	}

	DynamicPrimitiveUniformBuffer.Set(
		GetLocalToWorld(),
		PreviousLocalToWorld,
		GetBounds(),
		GetLocalBounds(),
		true,
		bHasPrecomputedVolumetricLightmap,
#if ENGINE_MINOR_VERSION < 23
		UseEditorDepthTest()
#else
		DrawsVelocity(), bOutputVelocity
#endif
	);
}

bool FVoxelProceduralMeshSceneProxy::ShouldDrawComplexCollisions(const FEngineShowFlags& EngineShowFlags) const
{
	if (IsCollisionEnabled())
	{
		// See if we have a response to the interested channel
		bool bHasResponse = EngineShowFlags.CollisionPawn && CollisionResponse.GetResponse(ECC_Pawn) != ECR_Ignore;
		bHasResponse |= EngineShowFlags.CollisionVisibility && CollisionResponse.GetResponse(ECC_Visibility) != ECR_Ignore;

		if (bHasResponse)
		{
			// Visibility uses complex and pawn uses simple. However, if UseSimpleAsComplex or UseComplexAsSimple is used we need to adjust accordingly
			return
				(EngineShowFlags.CollisionVisibility && CollisionTraceFlag != ECollisionTraceFlag::CTF_UseSimpleAsComplex) ||
				(EngineShowFlags.CollisionPawn && CollisionTraceFlag == ECollisionTraceFlag::CTF_UseComplexAsSimple);
		}
	}
	return false;
}