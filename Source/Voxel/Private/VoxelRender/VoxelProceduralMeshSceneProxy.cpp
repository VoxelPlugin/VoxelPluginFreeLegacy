// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelProceduralMeshSceneProxy.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelGlobals.h"

#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"
#if ENABLE_TESSELLATION
#include "TessellationRendering.h"
#endif

void FVoxelProcMeshProxySection::InitResources()
{
	BeginInitResource(&VertexBuffers.PositionVertexBuffer);
	BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
	BeginInitResource(&VertexBuffers.ColorVertexBuffer);
	BeginInitResource(&IndexBuffer);
	if (bRequiresAdjacencyInformation)
	{
		BeginInitResource(&AdjacencyIndexBuffer);
	}

	ENQUEUE_RENDER_COMMAND(InitVoxelMeshVertexFactory)(
		[this](FRHICommandListImmediate& RHICmdList)
	{
		auto& VB = VertexBuffers;
		auto& VF = VertexFactory;
		FLocalVertexFactory::FDataType Data;
		VB.PositionVertexBuffer.BindPositionVertexBuffer(&VF, Data);
		VB.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VF, Data);
		VB.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&VF, Data);
		VB.ColorVertexBuffer.BindColorVertexBuffer(&VF, Data);
		VF.SetData(Data);
		VF.InitResource();
	});
}

void FVoxelProcMeshProxySection::ReleaseResources()
{
	check(IsInRenderingThread());
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	VertexBuffers.ColorVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
	if (bRequiresAdjacencyInformation)
	{
		AdjacencyIndexBuffer.ReleaseResource();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

DECLARE_CYCLE_STAT(TEXT("FVoxelProceduralMeshSceneProxy Constructor")             , STAT_FVoxelProceduralMeshSceneProxy_FVoxelProceduralMeshSceneProxy            , STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelProceduralMeshSceneProxy Constructor Copy Section"), STAT_FVoxelProceduralMeshSceneProxy_FVoxelProceduralMeshSceneProxy_CopySection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelProceduralMeshSceneProxy::GetDynamicMeshElements") , STAT_FVoxelProceduralMeshSceneProxy_GetDynamicMeshElements                    , STATGROUP_Voxel);

FVoxelProceduralMeshSceneProxy::FVoxelProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component)
	: FPrimitiveSceneProxy(Component)
	, BodySetup(Component->GetBodySetup())
	, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelProceduralMeshSceneProxy_FVoxelProceduralMeshSceneProxy);

	// Copy each section
	const int32 NumSections = Component->ProcMeshSections.Num();
	Sections.AddZeroed(NumSections);
	for (int SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
	{
		FVoxelProcMeshSection& SrcSection = Component->ProcMeshSections[SectionIdx];
		if (SrcSection.Indices.Num() > 0 && SrcSection.Positions.Num() > 0)
		{
			FVoxelProcMeshProxySection* NewSection = new FVoxelProcMeshProxySection(GetScene().GetFeatureLevel());
						
			// Grab material
			NewSection->Material = SrcSection.Material ? SrcSection.Material : Component->GetMaterial(SectionIdx);
			if (!NewSection->Material)
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}
			check(NewSection->Material->IsValidLowLevel());

			// Copy vertices & indices
			{
				SCOPE_CYCLE_COUNTER(STAT_FVoxelProceduralMeshSceneProxy_FVoxelProceduralMeshSceneProxy_CopySection);

				int VertexCount = SrcSection.Positions.Num();
				bool bNeeds32BitIndices = VertexCount > MAX_uint16;
				NewSection->IndexBuffer.SetIndices(SrcSection.Indices, bNeeds32BitIndices ? EIndexBufferStride::Force32Bit : EIndexBufferStride::Force16Bit);

				auto& PositionBuffer = NewSection->VertexBuffers.PositionVertexBuffer;
				auto& ColorBuffer = NewSection->VertexBuffers.ColorVertexBuffer;
				auto& SMBuffer = NewSection->VertexBuffers.StaticMeshVertexBuffer;

				PositionBuffer.Init(SrcSection.Positions);
				ColorBuffer.InitFromColorArray(SrcSection.Colors);

				SMBuffer.Init(VertexCount, 1);
				for (int I = 0; I < VertexCount; I++)
				{
					auto& Tangent = SrcSection.Tangents[I];
					auto& Normal = SrcSection.Normals[I];
					auto& TextureCoordinate = SrcSection.TextureCoordinates[I];
					SMBuffer.SetVertexTangents(I, Tangent.TangentX, Tangent.GetY(Normal), Normal);
					SMBuffer.SetVertexUV(I, 0, TextureCoordinate);
				}
#if ENABLE_TESSELLATION
				NewSection->bRequiresAdjacencyInformation = RequiresAdjacencyInformation(NewSection->Material, NewSection->VertexFactory.GetType(), GetScene().GetFeatureLevel()) && SrcSection.AdjacencyIndices.Num() > 0;

				if (NewSection->bRequiresAdjacencyInformation)
				{
					NewSection->AdjacencyIndexBuffer.SetIndices(SrcSection.AdjacencyIndices, bNeeds32BitIndices ? EIndexBufferStride::Force32Bit : EIndexBufferStride::Force16Bit);
				}
#else 
				NewSection->bRequiresAdjacencyInformation = false;
#endif
			}
			// Enqueue initialization of render resource
			NewSection->InitResources();			

			// Copy visibility info
			NewSection->bSectionVisible = SrcSection.bSectionVisible;

			// Save ref to new section
			Sections[SectionIdx] = NewSection;
		}
	}
}

FVoxelProceduralMeshSceneProxy::~FVoxelProceduralMeshSceneProxy()
{
	for (FVoxelProcMeshProxySection* Section : Sections)
	{
		if (Section != nullptr)
		{
			Section->ReleaseResources();
			delete Section;
		}
	}
}

void FVoxelProceduralMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView *>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelProceduralMeshSceneProxy_GetDynamicMeshElements);
	
	// Iterate over sections
	for (FVoxelProcMeshProxySection* Section : Sections)
	{
		if (Section != nullptr && Section->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = Section->Material->GetRenderProxy(false);

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					Mesh.VertexFactory = &Section->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = true;
					Mesh.bUseWireframeSelectionColoring = IsSelected();

					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					BatchElement.PrimitiveUniformBufferResource = &GetUniformBuffer();
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.GetNumIndices() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

#if ENABLE_TESSELLATION
					if (Section->IndexBuffer.GetNumIndices() != 0 && Section->bRequiresAdjacencyInformation)
					{
						BatchElement.IndexBuffer = &Section->AdjacencyIndexBuffer;
						Mesh.Type = PT_12_ControlPointPatchList;
						BatchElement.FirstIndex *= 4;
					}
#endif

					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	// Draw bounds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			// Draw simple collision as wireframe if 'show collision', and collision is enabled, and we are not using the complex as the simple
			if (ViewFamily.EngineShowFlags.Collision && IsCollisionEnabled() && BodySetup->GetCollisionTraceFlag() != ECollisionTraceFlag::CTF_UseComplexAsSimple)
			{
				FTransform GeomTransform(GetLocalToWorld());
				BodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(FColor(157, 149, 223, 255), IsSelected(), IsHovered()).ToFColor(true), NULL, false, false, UseEditorDepthTest(), ViewIndex, Collector);
			}

			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif
}

FPrimitiveViewRelevance FVoxelProceduralMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
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

uint32 FVoxelProceduralMeshSceneProxy::GetAllocatedSize() const
{
	return FPrimitiveSceneProxy::GetAllocatedSize();
}
