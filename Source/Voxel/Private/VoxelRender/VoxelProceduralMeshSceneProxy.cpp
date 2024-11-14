// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/VoxelProceduralMeshSceneProxy.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelMinimal.h"

#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"
#include "DistanceFieldAtlas.h"
#include "PrimitiveSceneInfo.h"

// Needed to cancel motion blur when reusing proxies
#include "Runtime/Renderer/Private/ScenePrivate.h"

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
	VOXEL_RENDER_FUNCTION_COUNTER();
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
	VertexFactory.SetData(UE_504_ONLY(FRHICommandListImmediate::Get(), ) Data);
	VertexFactory.InitResource(UE_503_ONLY(FRHICommandListImmediate::Get()));

#if RHI_RAYTRACING
	if (IsRayTracingEnabled())
	{
		FRayTracingGeometryInitializer Initializer;
		Initializer.IndexBuffer = IndexBuffer.IndexBufferRHI;
		Initializer.TotalPrimitiveCount = IndexBuffer.GetNumIndices() / 3;
		Initializer.GeometryType = RTGT_Triangles;
		Initializer.bFastBuild = true;
		Initializer.bAllowUpdate = false;

		FRayTracingGeometrySegment Segment;
		Segment.VertexBuffer = VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
		Segment.NumPrimitives = Initializer.TotalPrimitiveCount;
		Segment.MaxVertices = VertexBuffers.PositionVertexBuffer.GetNumVertices();
		Initializer.Segments.Add(Segment);

		RayTracingGeometry.SetInitializer(Initializer);
		RayTracingGeometry.InitResource(UE_503_ONLY(FRHICommandListImmediate::Get()));
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
	VOXEL_RENDER_FUNCTION_COUNTER();
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
	, WeakToolRenderingManager(Component->ToolRenderingManager)
	, CollisionResponse(Component->GetCollisionResponseToChannels())
	, CollisionTraceFlag(Component->CollisionTraceFlag)
{
	VOXEL_FUNCTION_COUNTER();

#if VOXEL_ENGINE_VERSION >= 504
	// We create render data on-demand, can't be on a background thread
	bSupportsParallelGDME = false;
#endif

	// Proxy settings
	bSupportsDistanceFieldRepresentation = true;
	DistanceFieldSelfShadowBias = Component->DistanceFieldSelfShadowBias;
	bVerifyUsedMaterials = false; // Fails with tool rendering

	FinishSectionsUpdatesTime = Component->LastFinishSectionsUpdatesTime;
	CreateSceneProxyTime = FPlatformTime::Seconds();

	// Copy distance field data
	DistanceFieldData = Component->DistanceFieldData;

	// Deformable will invalidate VSMs
	bHasDeformableMesh = false;

	EnableGPUSceneSupportFlags();

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

		const bool bTessellatedMaterial = false;
		const bool bHasAdjacency = AdjacencyIndexBuffer.GetNumIndices() > 0;

		ensure(SrcSection.Settings.bEnableTessellation == bHasAdjacency);
		ensureMsgf(bTessellatedMaterial == bHasAdjacency, TEXT("Invalid tessellated material or non tessellated material is tessellated"));
		NewSection.bRequiresAdjacencyInformation = bTessellatedMaterial && bHasAdjacency;
	}

	{
		// Hack to cancel motion blur when mesh components are reused in the same frame
		const FMatrix PreviousLocalToWorld = Component->GetRenderMatrix();
		ENQUEUE_RENDER_COMMAND(UpdateTransformCommand)(
			[this, PreviousLocalToWorld](FRHICommandListImmediate& RHICmdList)
			{
				FScene& Scene = static_cast<FScene&>(GetScene());
				Scene.VelocityData.OverridePreviousTransform(GetPrimitiveComponentId(), PreviousLocalToWorld);
			});
	}
}

FVoxelProceduralMeshSceneProxy::~FVoxelProceduralMeshSceneProxy()
{
	for (auto& Section : Sections)
	{
		check(!Section.RenderData.IsValid());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelProceduralMeshSceneProxy::CreateRenderThreadResources(UE_504_ONLY(FRHICommandListBase& RHICmdList))
{
	VOXEL_RENDER_FUNCTION_COUNTER();
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
}

void FVoxelProceduralMeshSceneProxy::DestroyRenderThreadResources()
{
	VOXEL_RENDER_FUNCTION_COUNTER();
	check(IsInRenderingThread());
	
	for (auto& Section : Sections)
	{
		Section.RenderData.Reset();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelProceduralMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	VOXEL_RENDER_FUNCTION_COUNTER();

	const auto& EngineShowFlags = ViewFamily.EngineShowFlags;
	
#if NOT_SHIPPING_NOR_TEST
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

	// Render bounds
	{
		VOXEL_SLOW_SCOPE_COUNTER("Render Bounds");
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				RenderBounds(Collector.GetPDI(ViewIndex), EngineShowFlags, GetBounds(), IsSelected());
			}
		}
	}
	
	if (IsCollisionView(EngineShowFlags))
	{
		if (ShouldDrawComplexCollisions(EngineShowFlags))
		{
			VOXEL_SLOW_SCOPE_COUNTER("Complex Collisons");
			for (auto& Section : Sections)
			{
				if (!Section.bEnableCollisions_Debug) continue;
				
				const FColor ComplexCollisionColor = FColor(0, 255, 255, 255);
				auto* MaterialProxy = new FColoredMaterialRenderProxy(GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(), ComplexCollisionColor);
				Collector.RegisterOneFrameMaterialProxy(MaterialProxy);

				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (!(VisibilityMap & (1 << ViewIndex))) continue;

					FMeshBatch& Mesh = Collector.AllocateMesh();
					DrawSection(Mesh, Section, MaterialProxy, false, EngineShowFlags.Wireframe);
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}
	else if (FVoxelDebugManager::ShowCollisionAndNavmeshDebug())
	{
		VOXEL_SLOW_SCOPE_COUNTER("Collision and Navmesh Debug");
		for (auto& Section : Sections)
		{
			if (!Section.RenderData.IsValid()) continue;
			
			const auto* ParentMaterial =
				EngineShowFlags.Wireframe
				? GEngine->WireframeMaterial.Get()
				: GEngine->LevelColorationLitMaterial.Get();

			const auto Color = FVoxelDebugManager::GetCollisionAndNavmeshDebugColor(Section.bEnableCollisions_Debug, Section.bEnableNavmesh_Debug);

			if (!ensure(ParentMaterial)) return; // Happens in packaged games
			
			auto* MaterialProxy = new FColoredMaterialRenderProxy(ParentMaterial->GetRenderProxy(), Color);
			Collector.RegisterOneFrameMaterialProxy(MaterialProxy);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (!(VisibilityMap & (1 << ViewIndex))) continue;

				FMeshBatch& Mesh = Collector.AllocateMesh();
				DrawSection(Mesh, Section, MaterialProxy, false, EngineShowFlags.Wireframe);
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
			VOXEL_SLOW_SCOPE_COUNTER("Render Section");
			
			if (!Section.bSectionVisible)
			{
				continue;
			}

			auto* Material = Section.Material->GetMaterial();
			if (!Material)
			{
				// Will happen in force delete
				continue;
			}
			
			auto* MaterialProxy = Material->GetRenderProxy();

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

				FMeshBatch& Mesh = Collector.AllocateMesh();
				DrawSection(Mesh, Section, MaterialProxy, !bForceDisableTessellation, EngineShowFlags.Wireframe);

				{
					VOXEL_SLOW_SCOPE_COUNTER("Collector.AddMesh");
					Collector.AddMesh(ViewIndex, Mesh);
				}

				INC_DWORD_STAT(STAT_NumVoxelDrawCalls);
				INC_DWORD_STAT_BY(STAT_NumVoxelTrianglesDrawn, Section.Buffers->GetNumIndices() / 3);
			}
		}
		
		const auto ToolRenderingManager = WeakToolRenderingManager.Pin();
		if (ToolRenderingManager.IsValid())
		{
			VOXEL_RENDER_SCOPE_COUNTER("Render Tools");

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
				UMaterialInterface* Material = nullptr;
				if (Tool.Material.IsValid())
				{
					Material = Tool.Material->GetMaterial();
				}
				if (!Material)
				{
					Material = FVoxelMaterialInterfaceManager::Get().DefaultMaterial()->GetMaterial();
				}
				if (!ensure(Material))
				{
					continue;
				}
				auto* MaterialProxy = Material->GetRenderProxy();

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

						FMeshBatch& Mesh = Collector.AllocateMesh();
						DrawSection(Mesh, Section, MaterialProxy, !bForceDisableTessellation, EngineShowFlags.Wireframe);
						Collector.AddMesh(ViewIndex, Mesh);

						INC_DWORD_STAT(STAT_NumVoxelDrawCallsForTools);
						INC_DWORD_STAT_BY(STAT_NumVoxelTrianglesDrawnForTools, Section.Buffers->GetNumIndices() / 3)
					}
				}
			}
		}
	}
}

void FVoxelProceduralMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	if (RuntimeVirtualTextureMaterialTypes.Num() == 0)
	{
		return;
	}

	for (const auto& Section : Sections)
	{
		auto* Material = Section.Material->GetMaterial();
		if (!Material)
		{
			// Will happen in force delete
			continue;
		}
			
		auto* MaterialProxy = Material->GetRenderProxy();

		FMeshBatch MeshBatch;
		DrawSection(MeshBatch, Section, MaterialProxy, false, false);

		// Else the virtual texture check fails in RuntimeVirtualTextureRender.cpp:338
		// and the static mesh isn't rendered at all
		MeshBatch.LODIndex = 0;

		// Runtime virtual texture mesh elements.
		MeshBatch.CastShadow = 0;
		MeshBatch.bUseAsOccluder = 0;
		MeshBatch.bUseForDepthPass = 0;
		MeshBatch.bUseForMaterial = 0;
		MeshBatch.bDitheredLODTransition = 0;
		MeshBatch.bRenderToVirtualTexture = 1;

		for (ERuntimeVirtualTextureMaterialType MaterialType : RuntimeVirtualTextureMaterialTypes)
		{
			MeshBatch.RuntimeVirtualTextureMaterialType = uint32(MaterialType);
			PDI->DrawMesh(MeshBatch, FLT_MAX);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if RHI_RAYTRACING
#if VOXEL_ENGINE_VERSION >= 505
void FVoxelProceduralMeshSceneProxy::GetDynamicRayTracingInstances(FRayTracingInstanceCollector& Collector)
#else
void FVoxelProceduralMeshSceneProxy::GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances)
#endif
{
	VOXEL_RENDER_FUNCTION_COUNTER();

	for (const auto& Section : Sections)
	{
		if (Section.bSectionVisible && ensure(Section.Material->GetMaterial()->IsValidLowLevel()))
		{
			auto& RenderData = *Section.RenderData;
#if VOXEL_ENGINE_VERSION >= 505
			if (RenderData.RayTracingGeometry.GetRHI())
#else
			if (RenderData.RayTracingGeometry.RayTracingGeometryRHI.IsValid())
#endif
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

				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				BatchElement.IndexBuffer = &Section.Buffers->IndexBuffer;
				
				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = Section.Buffers->IndexBuffer.GetNumIndices() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = Section.Buffers->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

				RayTracingInstance.Materials.Add(MeshBatch);

#if VOXEL_ENGINE_VERSION >= 505
				Collector.AddRayTracingInstance(RayTracingInstance);
#else
				OutRayTracingInstances.Add(RayTracingInstance);
#endif
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
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
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

void FVoxelProceduralMeshSceneProxy::DrawSection(
	FMeshBatch& Mesh,
	const FVoxelProcMeshProxySection& Section,
	const FMaterialRenderProxy* MaterialRenderProxy,
	bool bEnableTessellation,
	bool bWireframe) const
{
	VOXEL_RENDER_FUNCTION_COUNTER();
	
	check(MaterialRenderProxy);
	check(Section.RenderData.IsValid());

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

	FMeshBatchElement& BatchElement = Mesh.Elements[0];
	BatchElement.IndexBuffer = &Section.Buffers->IndexBuffer;
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = Section.Buffers->IndexBuffer.GetNumIndices() / 3;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = Section.Buffers->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

#if ENABLE_TESSELLATION
	if (bEnableTessellation)
	{
		// Could be different from bRequiresAdjacencyInformation during shader compilation
		const bool bCurrentRequiresAdjacencyInformation = false;

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