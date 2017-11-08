// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved. 

#include "VoxelPrivatePCH.h"
#include "VoxelProceduralMeshComponent.h"
#include "PrimitiveViewRelevance.h"
#include "RenderResource.h"
#include "RenderingThread.h"
#include "PrimitiveSceneProxy.h"
#include "Containers/ResourceArray.h"
#include "EngineGlobals.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "Engine/Engine.h"
#include "SceneManagement.h"
#include "PhysicsEngine/BodySetup.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Stats/Stats.h"
#include "nvtess.h"
#include "PositionVertexBuffer.h"
#include "StaticMeshVertexBuffer.h"
#include <TessellationRendering.h>

DECLARE_STATS_GROUP(TEXT("ProceduralMesh"), STATGROUP_ProceduralMesh, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Create ProcMesh Proxy"), STAT_ProcMesh_CreateSceneProxy, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Create Mesh Section"), STAT_ProcMesh_CreateMeshSection, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("UpdateSection GT"), STAT_ProcMesh_UpdateSectionGT, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("UpdateSection RT"), STAT_ProcMesh_UpdateSectionRT, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Get ProcMesh Elements"), STAT_ProcMesh_GetMeshElements, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Update Collision"), STAT_ProcMesh_UpdateCollision, STATGROUP_ProceduralMesh);





/** Resource array to pass  */
class FProcMeshVertexResourceArray : public FResourceArrayInterface
{
public:
	FProcMeshVertexResourceArray(void* InData, uint32 InSize)
		: Data(InData)
		, Size(InSize)
	{
	}

	virtual const void* GetResourceData() const override { return Data; }
	virtual uint32 GetResourceDataSize() const override { return Size; }
	virtual void Discard() override {}
	virtual bool IsStatic() const override { return false; }
	virtual bool GetAllowCPUAccess() const override { return false; }
	virtual void SetAllowCPUAccess(bool bInNeedsCPUAccess) override {}

private:
	void* Data;
	uint32 Size;
};

/** Vertex Buffer */
class FProcMeshVertexBuffer : public FVertexBuffer
{
public:
	TArray<FDynamicMeshVertex> Vertices;

	virtual void InitRHI() override
	{
		const uint32 SizeInBytes = Vertices.Num() * sizeof(FDynamicMeshVertex);

		FProcMeshVertexResourceArray ResourceArray(Vertices.GetData(), SizeInBytes);
		FRHIResourceCreateInfo CreateInfo(&ResourceArray);
		VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Static, CreateInfo);
	}

};

/** Index Buffer */
class FProcMeshIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		void* Buffer = nullptr;
		IndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo, Buffer);

		// Write the indices to the index buffer.		
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

/** Vertex Factory */
class FProcMeshVertexFactory : public FLocalVertexFactory
{
public:

	FProcMeshVertexFactory()
	{
	}

	/** Init function that should only be called on render thread. */
	void Init_RenderThread(const FProcMeshVertexBuffer* VertexBuffer)
	{
		check(IsInRenderingThread());

		// Initialize the vertex factory's stream components.
		FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(
			FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
		);
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
		SetData(NewData);
	}

	/** Init function that can be called on any thread, and will do the right thing (enqueue command if called on main thread) */
	void Init(const FProcMeshVertexBuffer* VertexBuffer)
	{
		if (IsInRenderingThread())
		{
			Init_RenderThread(VertexBuffer);
		}
		else
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				InitProcMeshVertexFactory,
				FProcMeshVertexFactory*, VertexFactory, this,
				const FProcMeshVertexBuffer*, VertexBuffer, VertexBuffer,
				{
				VertexFactory->Init_RenderThread(VertexBuffer);
				});
		}
	}
};

/** Class representing a single section of the proc mesh */
class FProcMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FProcMeshVertexBuffer VertexBuffer;
	/** Index buffer for this section */
	FProcMeshIndexBuffer IndexBuffer;
	/** Vertex factory for this section */
	FProcMeshVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible;

	bool bRequiresAdjacencyInformation;


	FProcMeshIndexBuffer AdjacencyIndexBuffer;

	FProcMeshProxySection()
		: Material(NULL)
		, bSectionVisible(true)
	{
	}
};

/**
 *	Struct used to send update to mesh data
 *	Arrays may be empty, in which case no update is performed.
 */
class FProcMeshSectionUpdateData
{
public:
	/** Section to update */
	int32 TargetSection;
	/** New vertex information */
	TArray<FVoxelProcMeshVertex> NewVertexBuffer;
};

static void ConvertProcMeshToDynMeshVertex(FDynamicMeshVertex& Vert, const FVoxelProcMeshVertex& ProcVert)
{
	Vert.Position = ProcVert.Position;
	Vert.Color = ProcVert.Color;
	Vert.TextureCoordinate = ProcVert.UV0;
	Vert.TangentX = ProcVert.Tangent.TangentX;
	Vert.TangentZ = ProcVert.Normal;
	Vert.TangentZ.Vector.W = ProcVert.Tangent.bFlipTangentY ? 0 : 255;
}














/*------------------------------------------------------------------------------
NVTessLib for computing adjacency used for tessellation.
------------------------------------------------------------------------------*/

/**
* Provides static mesh render data to the NVIDIA tessellation library.
*/
class FStaticMeshNvRenderBuffer : public nv::RenderBuffer
{
public:

	/** Construct from static mesh render buffers. */
	FStaticMeshNvRenderBuffer(
		const FProcMeshVertexBuffer& InVertexBuffer,
		const TArray<uint32>& Indices)
		: VertexBuffer(InVertexBuffer)
	{
		mIb = new nv::IndexBuffer((void*)Indices.GetData(), nv::IBT_U32, Indices.Num(), false);
	}

	/** Retrieve the position and first texture coordinate of the specified index. */
	virtual nv::Vertex getVertex(unsigned int Index) const
	{
		nv::Vertex Vertex;

		const FVector& Position = VertexBuffer.Vertices[Index].Position;
		Vertex.pos.x = Position.X;
		Vertex.pos.y = Position.Y;
		Vertex.pos.z = Position.Z;

		Vertex.uv.x = 0.0f;
		Vertex.uv.y = 0.0f;

		return Vertex;
	}

private:

	/** The position vertex buffer for the static mesh. */
	const FProcMeshVertexBuffer& VertexBuffer;

	/** Copying is forbidden. */
	FStaticMeshNvRenderBuffer(const FStaticMeshNvRenderBuffer&);
	FStaticMeshNvRenderBuffer& operator=(const FStaticMeshNvRenderBuffer&);
};


static void BuildStaticAdjacencyIndexBuffer(
	const FProcMeshVertexBuffer& PositionVertexBuffer,
	const TArray<uint32>& Indices,
	TArray<int32>& OutPnAenIndices
)
{
	if (Indices.Num())
	{
		FStaticMeshNvRenderBuffer StaticMeshRenderBuffer(PositionVertexBuffer, Indices);
		nv::IndexBuffer* PnAENIndexBuffer = nv::tess::buildTessellationBuffer(&StaticMeshRenderBuffer, nv::DBM_PnAenDominantCorner, true);
		check(PnAENIndexBuffer);
		const int32 IndexCount = (int32)PnAENIndexBuffer->getLength();
		OutPnAenIndices.Empty(IndexCount);
		OutPnAenIndices.AddUninitialized(IndexCount);
		for (int32 Index = 0; Index < IndexCount; ++Index)
		{
			OutPnAenIndices[Index] = (*PnAENIndexBuffer)[Index];
		}
		delete PnAENIndexBuffer;
	}
	else
	{
		OutPnAenIndices.Empty();
	}
}
















/** Procedural mesh scene proxy */
class FProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, BodySetup(Component->GetBodySetup())
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		// Copy each section
		const int32 NumSections = Component->ProcMeshSections.Num();
		Sections.AddZeroed(NumSections);
		for (int SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
		{
			FVoxelProcMeshSection& SrcSection = Component->ProcMeshSections[SectionIdx];
			if (SrcSection.ProcIndexBuffer.Num() > 0 && SrcSection.ProcVertexBuffer.Num() > 0)
			{
				FProcMeshProxySection* NewSection = new FProcMeshProxySection();

				// Copy data from vertex buffer
				const int32 NumVerts = SrcSection.ProcVertexBuffer.Num();

				// Allocate verts
				NewSection->VertexBuffer.Vertices.SetNumUninitialized(NumVerts);
				// Copy verts
				for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
				{
					const FVoxelProcMeshVertex& ProcVert = SrcSection.ProcVertexBuffer[VertIdx];
					FDynamicMeshVertex& Vert = NewSection->VertexBuffer.Vertices[VertIdx];
					ConvertProcMeshToDynMeshVertex(Vert, ProcVert);
				}

				// Copy index buffer
				NewSection->IndexBuffer.Indices = SrcSection.ProcIndexBuffer;

				// Init vertex factory
				NewSection->VertexFactory.Init(&NewSection->VertexBuffer);



				TArray<uint32> Indices;
				Indices.SetNum(NewSection->IndexBuffer.Indices.Num());
				for (int i = 0; i < Indices.Num(); i++)
				{
					Indices[i] = NewSection->IndexBuffer.Indices[i];
				}

				BuildStaticAdjacencyIndexBuffer(
					NewSection->VertexBuffer,
					Indices,
					NewSection->AdjacencyIndexBuffer.Indices
				);


				// Enqueue initialization of render resource
				BeginInitResource(&NewSection->VertexBuffer);
				BeginInitResource(&NewSection->IndexBuffer);
				BeginInitResource(&NewSection->VertexFactory);
				BeginInitResource(&NewSection->AdjacencyIndexBuffer);

				// Grab material
				NewSection->Material = Component->GetMaterial(SectionIdx);
				if (NewSection->Material == NULL)
				{
					NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
				}

				NewSection->bRequiresAdjacencyInformation = RequiresAdjacencyInformation(NewSection->Material, NewSection->VertexFactory.GetType(), GetScene().GetFeatureLevel());

				// Copy visibility info
				NewSection->bSectionVisible = SrcSection.bSectionVisible;

				// Save ref to new section
				Sections[SectionIdx] = NewSection;
			}
		}
	}

	virtual ~FProceduralMeshSceneProxy()
	{
		for (FProcMeshProxySection* Section : Sections)
		{
			if (Section != nullptr)
			{
				Section->VertexBuffer.ReleaseResource();
				Section->IndexBuffer.ReleaseResource();
				Section->VertexFactory.ReleaseResource();
				Section->AdjacencyIndexBuffer.ReleaseResource();
				delete Section;
			}
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FProcMeshSectionUpdateData* SectionData)
	{
		SCOPE_CYCLE_COUNTER(STAT_ProcMesh_UpdateSectionRT);

		check(IsInRenderingThread());

		// Check we have data 
		if (SectionData != nullptr)
		{
			// Check it references a valid section
			if (SectionData->TargetSection < Sections.Num() &&
				Sections[SectionData->TargetSection] != nullptr)
			{
				FProcMeshProxySection* Section = Sections[SectionData->TargetSection];

				// Lock vertex buffer
				const int32 NumVerts = SectionData->NewVertexBuffer.Num();
				FDynamicMeshVertex* VertexBufferData = (FDynamicMeshVertex*)RHILockVertexBuffer(Section->VertexBuffer.VertexBufferRHI, 0, NumVerts * sizeof(FDynamicMeshVertex), RLM_WriteOnly);

				// Iterate through vertex data, copying in new info
				for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
				{
					const FVoxelProcMeshVertex& ProcVert = SectionData->NewVertexBuffer[VertIdx];
					FDynamicMeshVertex& Vert = VertexBufferData[VertIdx];
					ConvertProcMeshToDynMeshVertex(Vert, ProcVert);
				}

				// Unlock vertex buffer
				RHIUnlockVertexBuffer(Section->VertexBuffer.VertexBufferRHI);
			}

			// Free data sent from game thread
			delete SectionData;
		}
	}

	void SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility)
	{
		check(IsInRenderingThread());

		if (SectionIndex < Sections.Num() &&
			Sections[SectionIndex] != nullptr)
		{
			Sections[SectionIndex]->bSectionVisible = bNewVisibility;
		}
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		SCOPE_CYCLE_COUNTER(STAT_ProcMesh_GetMeshElements);


		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = NULL;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		// Iterate over sections
		for (FProcMeshProxySection* Section : Sections)
		{
			if (Section != nullptr && Section->bSectionVisible)
			{
				FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy(IsSelected());

				// For each view..
				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (VisibilityMap & (1 << ViewIndex))
					{
						const FSceneView* View = Views[ViewIndex];
						// Draw the mesh.
						FMeshBatch& Mesh = Collector.AllocateMesh();
						FMeshBatchElement& BatchElement = Mesh.Elements[0];
						BatchElement.IndexBuffer = &Section->IndexBuffer;
						Mesh.bWireframe = bWireframe;
						Mesh.VertexFactory = &Section->VertexFactory;
						Mesh.MaterialRenderProxy = MaterialProxy;
						BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
						BatchElement.FirstIndex = 0;
						BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
						BatchElement.MinVertexIndex = 0;
						BatchElement.MaxVertexIndex = Section->VertexBuffer.Vertices.Num() - 1;
						Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
						Mesh.Type = PT_TriangleList;
						Mesh.DepthPriorityGroup = SDPG_World;
						Mesh.bCanApplyViewModeOverrides = false;

						if (Section->IndexBuffer.Indices.Num() != 0 && !bWireframe && Section->bRequiresAdjacencyInformation)
						{
							BatchElement.IndexBuffer = &Section->AdjacencyIndexBuffer;
							Mesh.Type = PT_12_ControlPointPatchList;
							BatchElement.FirstIndex *= 4;
						}

						if (bWireframe)
						{
							Mesh.bWireframe = true;
							Mesh.bDisableBackfaceCulling = true;
						}

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

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
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

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	/** Array of sections */
	TArray<FProcMeshProxySection*> Sections;

	UBodySetup* BodySetup;

	FMaterialRelevance MaterialRelevance;
};


//////////////////////////////////////////////////////////////////////////


UVoxelProceduralMeshComponent::UVoxelProceduralMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseComplexAsSimpleCollision = true;
}

void UVoxelProceduralMeshComponent::PostLoad()
{
	Super::PostLoad();

	if (ProcMeshBodySetup && IsTemplate())
	{
		ProcMeshBodySetup->SetFlags(RF_Public);
	}
}

void UVoxelProceduralMeshComponent::CreateMeshSection_LinearColor(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FLinearColor>& VertexColors, const TArray<FVoxelProcMeshTangent>& Tangents, bool bCreateCollision)
{
	// Convert FLinearColors to FColors
	TArray<FColor> Colors;
	if (VertexColors.Num() > 0)
	{
		Colors.SetNum(VertexColors.Num());

		for (int32 ColorIdx = 0; ColorIdx < VertexColors.Num(); ColorIdx++)
		{
			Colors[ColorIdx] = VertexColors[ColorIdx].ToFColor(false);
		}
	}

	CreateMeshSection(SectionIndex, Vertices, Triangles, Normals, UV0, Colors, Tangents, bCreateCollision);
}

void UVoxelProceduralMeshComponent::CreateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors, const TArray<FVoxelProcMeshTangent>& Tangents, bool bCreateCollision)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcMesh_CreateMeshSection);

	// Ensure sections array is long enough
	if (SectionIndex >= ProcMeshSections.Num())
	{
		ProcMeshSections.SetNum(SectionIndex + 1, false);
	}

	// Reset this section (in case it already existed)
	FVoxelProcMeshSection& NewSection = ProcMeshSections[SectionIndex];
	NewSection.Reset();

	// Copy data to vertex buffer
	const int32 NumVerts = Vertices.Num();
	NewSection.ProcVertexBuffer.Reset();
	NewSection.ProcVertexBuffer.AddUninitialized(NumVerts);
	for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
	{
		FVoxelProcMeshVertex& Vertex = NewSection.ProcVertexBuffer[VertIdx];

		Vertex.Position = Vertices[VertIdx];
		Vertex.Normal = (Normals.Num() == NumVerts) ? Normals[VertIdx] : FVector(0.f, 0.f, 1.f);
		Vertex.UV0 = (UV0.Num() == NumVerts) ? UV0[VertIdx] : FVector2D(0.f, 0.f);
		Vertex.Color = (VertexColors.Num() == NumVerts) ? VertexColors[VertIdx] : FColor(255, 255, 255);
		Vertex.Tangent = (Tangents.Num() == NumVerts) ? Tangents[VertIdx] : FVoxelProcMeshTangent();

		// Update bounding box
		NewSection.SectionLocalBox += Vertex.Position;
	}

	// Copy index buffer (clamping to vertex range)
	int32 NumTriIndices = Triangles.Num();
	NumTriIndices = (NumTriIndices / 3) * 3; // Ensure we have exact number of triangles (array is multiple of 3 long)

	NewSection.ProcIndexBuffer.Reset();
	NewSection.ProcIndexBuffer.AddUninitialized(NumTriIndices);
	for (int32 IndexIdx = 0; IndexIdx < NumTriIndices; IndexIdx++)
	{
		NewSection.ProcIndexBuffer[IndexIdx] = FMath::Min(Triangles[IndexIdx], NumVerts - 1);
	}

	NewSection.bEnableCollision = bCreateCollision;

	UpdateLocalBounds(); // Update overall bounds
	UpdateCollision(); // Mark collision as dirty
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

void UVoxelProceduralMeshComponent::UpdateMeshSection_LinearColor(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FLinearColor>& VertexColors, const TArray<FVoxelProcMeshTangent>& Tangents)
{
	// Convert FLinearColors to FColors
	TArray<FColor> Colors;
	if (VertexColors.Num() > 0)
	{
		Colors.SetNum(VertexColors.Num());

		for (int32 ColorIdx = 0; ColorIdx < VertexColors.Num(); ColorIdx++)
		{
			Colors[ColorIdx] = VertexColors[ColorIdx].ToFColor(true);
		}
	}

	UpdateMeshSection(SectionIndex, Vertices, Normals, UV0, Colors, Tangents);
}

void UVoxelProceduralMeshComponent::UpdateMeshSection(int32 SectionIndex, const TArray<FVector>& Vertices, const TArray<FVector>& Normals, const TArray<FVector2D>& UV0, const TArray<FColor>& VertexColors, const TArray<FVoxelProcMeshTangent>& Tangents)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcMesh_UpdateSectionGT);

	if (SectionIndex < ProcMeshSections.Num())
	{
		FVoxelProcMeshSection& Section = ProcMeshSections[SectionIndex];
		const int32 NumVerts = Section.ProcVertexBuffer.Num();

		// See if positions are changing
		const bool bPositionsChanging = (Vertices.Num() == NumVerts);

		// Update bounds, if we are getting new position data
		if (bPositionsChanging)
		{
			Section.SectionLocalBox.Init();
		}

		// Iterate through vertex data, copying in new info
		for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
		{
			FVoxelProcMeshVertex& ModifyVert = Section.ProcVertexBuffer[VertIdx];

			// Position data
			if (Vertices.Num() == NumVerts)
			{
				ModifyVert.Position = Vertices[VertIdx];
				Section.SectionLocalBox += ModifyVert.Position;
			}

			// Normal data
			if (Normals.Num() == NumVerts)
			{
				ModifyVert.Normal = Normals[VertIdx];
			}

			// Tangent data 
			if (Tangents.Num() == NumVerts)
			{
				ModifyVert.Tangent = Tangents[VertIdx];
			}

			// UV data
			if (UV0.Num() == NumVerts)
			{
				ModifyVert.UV0 = UV0[VertIdx];
			}

			// Color data
			if (VertexColors.Num() == NumVerts)
			{
				ModifyVert.Color = VertexColors[VertIdx];
			}
		}

		if (SceneProxy)
		{
			// Create data to update section
			FProcMeshSectionUpdateData* SectionData = new FProcMeshSectionUpdateData;
			SectionData->TargetSection = SectionIndex;
			SectionData->NewVertexBuffer = Section.ProcVertexBuffer;

			// Enqueue command to send to render thread
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				FProcMeshSectionUpdate,
				FProceduralMeshSceneProxy*, ProcMeshSceneProxy, (FProceduralMeshSceneProxy*)SceneProxy,
				FProcMeshSectionUpdateData*, SectionData, SectionData,
				{
					ProcMeshSceneProxy->UpdateSection_RenderThread(SectionData);
				}
			);
		}

		// If we have collision enabled on this section, update that too
		if (bPositionsChanging && Section.bEnableCollision)
		{
			TArray<FVector> CollisionPositions;

			// We have one collision mesh for all sections, so need to build array of _all_ positions
			for (const FVoxelProcMeshSection& CollisionSection : ProcMeshSections)
			{
				// If section has collision, copy it
				if (CollisionSection.bEnableCollision)
				{
					for (int32 VertIdx = 0; VertIdx < CollisionSection.ProcVertexBuffer.Num(); VertIdx++)
					{
						CollisionPositions.Add(CollisionSection.ProcVertexBuffer[VertIdx].Position);
					}
				}
			}

			// Pass new positions to trimesh
			BodyInstance.UpdateTriMeshVertices(CollisionPositions);
		}

		if (bPositionsChanging)
		{
			UpdateLocalBounds(); // Update overall bounds
			MarkRenderTransformDirty(); // Need to send new bounds to render thread
		}
	}
}

void UVoxelProceduralMeshComponent::ClearMeshSection(int32 SectionIndex)
{
	if (SectionIndex < ProcMeshSections.Num())
	{
		ProcMeshSections[SectionIndex].Reset();
		UpdateLocalBounds();
		UpdateCollision();
		MarkRenderStateDirty();
	}
}

void UVoxelProceduralMeshComponent::ClearAllMeshSections()
{
	ProcMeshSections.Empty();
	UpdateLocalBounds();
	UpdateCollision();
	MarkRenderStateDirty();
}

void UVoxelProceduralMeshComponent::SetMeshSectionVisible(int32 SectionIndex, bool bNewVisibility)
{
	if (SectionIndex < ProcMeshSections.Num())
	{
		// Set game thread state
		ProcMeshSections[SectionIndex].bSectionVisible = bNewVisibility;

		if (SceneProxy)
		{
			// Enqueue command to modify render thread info
			ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
				FProcMeshSectionVisibilityUpdate,
				FProceduralMeshSceneProxy*, ProcMeshSceneProxy, (FProceduralMeshSceneProxy*)SceneProxy,
				int32, SectionIndex, SectionIndex,
				bool, bNewVisibility, bNewVisibility,
				{
					ProcMeshSceneProxy->SetSectionVisibility_RenderThread(SectionIndex, bNewVisibility);
				}
			);
		}
	}
}

bool UVoxelProceduralMeshComponent::IsMeshSectionVisible(int32 SectionIndex) const
{
	return (SectionIndex < ProcMeshSections.Num()) ? ProcMeshSections[SectionIndex].bSectionVisible : false;
}

int32 UVoxelProceduralMeshComponent::GetNumSections() const
{
	return ProcMeshSections.Num();
}

void UVoxelProceduralMeshComponent::AddCollisionConvexMesh(TArray<FVector> ConvexVerts)
{
	if (ConvexVerts.Num() >= 4)
	{
		// New element
		FKConvexElem NewConvexElem;
		// Copy in vertex info
		NewConvexElem.VertexData = ConvexVerts;
		// Update bounding box
		NewConvexElem.ElemBox = FBox(NewConvexElem.VertexData);
		// Add to array of convex elements
		CollisionConvexElems.Add(NewConvexElem);
		// Refresh collision
		UpdateCollision();
	}
}

void UVoxelProceduralMeshComponent::ClearCollisionConvexMeshes()
{
	// Empty simple collision info
	CollisionConvexElems.Empty();
	// Refresh collision
	UpdateCollision();
}

void UVoxelProceduralMeshComponent::SetCollisionConvexMeshes(const TArray< TArray<FVector> >& ConvexMeshes)
{
	CollisionConvexElems.Reset();

	// Create element for each convex mesh
	for (int32 ConvexIndex = 0; ConvexIndex < ConvexMeshes.Num(); ConvexIndex++)
	{
		FKConvexElem NewConvexElem;
		NewConvexElem.VertexData = ConvexMeshes[ConvexIndex];
		NewConvexElem.ElemBox = FBox(NewConvexElem.VertexData);

		CollisionConvexElems.Add(NewConvexElem);
	}

	UpdateCollision();
}


void UVoxelProceduralMeshComponent::UpdateLocalBounds()
{
	FBox LocalBox(ForceInit);

	for (const FVoxelProcMeshSection& Section : ProcMeshSections)
	{
		LocalBox += Section.SectionLocalBox;
	}

	LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0); // fallback to reset box sphere bounds

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

FPrimitiveSceneProxy* UVoxelProceduralMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_ProcMesh_CreateSceneProxy);

	return new FProceduralMeshSceneProxy(this);
}

int32 UVoxelProceduralMeshComponent::GetNumMaterials() const
{
	return ProcMeshSections.Num();
}


FVoxelProcMeshSection* UVoxelProceduralMeshComponent::GetProcMeshSection(int32 SectionIndex)
{
	if (SectionIndex < ProcMeshSections.Num())
	{
		return &ProcMeshSections[SectionIndex];
	}
	else
	{
		return nullptr;
	}
}


void UVoxelProceduralMeshComponent::SetProcMeshSection(int32 SectionIndex, const FVoxelProcMeshSection& Section)
{
	// Ensure sections array is long enough
	if (SectionIndex >= ProcMeshSections.Num())
	{
		ProcMeshSections.SetNum(SectionIndex + 1, false);
	}

	ProcMeshSections[SectionIndex] = Section;

	UpdateLocalBounds(); // Update overall bounds
	UpdateCollision(); // Mark collision as dirty
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

FBoxSphereBounds UVoxelProceduralMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}

bool UVoxelProceduralMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	int32 VertexBase = 0; // Base vertex index for current section

	// See if we should copy UVs
	bool bCopyUVs = UPhysicsSettings::Get()->bSupportUVFromHitResults;
	if (bCopyUVs)
	{
		CollisionData->UVs.AddZeroed(1); // only one UV channel
	}

	// For each section..
	for (int32 SectionIdx = 0; SectionIdx < ProcMeshSections.Num(); SectionIdx++)
	{
		FVoxelProcMeshSection& Section = ProcMeshSections[SectionIdx];
		// Do we have collision enabled?
		if (Section.bEnableCollision)
		{
			// Copy vert data
			for (int32 VertIdx = 0; VertIdx < Section.ProcVertexBuffer.Num(); VertIdx++)
			{
				CollisionData->Vertices.Add(Section.ProcVertexBuffer[VertIdx].Position);

				// Copy UV if desired
				if (bCopyUVs)
				{
					CollisionData->UVs[0].Add(Section.ProcVertexBuffer[VertIdx].UV0);
				}
			}

			// Copy triangle data
			const int32 NumTriangles = Section.ProcIndexBuffer.Num() / 3;
			for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
			{
				// Need to add base offset for indices
				FTriIndices Triangle;
				Triangle.v0 = Section.ProcIndexBuffer[(TriIdx * 3) + 0] + VertexBase;
				Triangle.v1 = Section.ProcIndexBuffer[(TriIdx * 3) + 1] + VertexBase;
				Triangle.v2 = Section.ProcIndexBuffer[(TriIdx * 3) + 2] + VertexBase;
				CollisionData->Indices.Add(Triangle);

				// Also store material info
				CollisionData->MaterialIndices.Add(SectionIdx);
			}

			// Remember the base index that new verts will be added from in next section
			VertexBase = CollisionData->Vertices.Num();
		}
	}

	CollisionData->bFlipNormals = true;
	CollisionData->bDeformableMesh = true;
	CollisionData->bFastCook = true;

	return true;
}

bool UVoxelProceduralMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	for (const FVoxelProcMeshSection& Section : ProcMeshSections)
	{
		if (Section.ProcIndexBuffer.Num() >= 3 && Section.bEnableCollision)
		{
			return true;
		}
	}

	return false;
}

UBodySetup* UVoxelProceduralMeshComponent::CreateBodySetupHelper()
{
	// The body setup in a template needs to be public since the property is Tnstanced and thus is the archetype of the instance meaning there is a direct reference
	UBodySetup* NewBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
	NewBodySetup->BodySetupGuid = FGuid::NewGuid();

	NewBodySetup->bGenerateMirroredCollision = false;
	NewBodySetup->bDoubleSidedGeometry = true;
	NewBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	return NewBodySetup;
}

void UVoxelProceduralMeshComponent::CreateProcMeshBodySetup()
{
	if (ProcMeshBodySetup == nullptr)
	{
		ProcMeshBodySetup = CreateBodySetupHelper();
	}
}

void UVoxelProceduralMeshComponent::UpdateCollision()
{
	SCOPE_CYCLE_COUNTER(STAT_ProcMesh_UpdateCollision);

	UWorld* World = GetWorld();
	const bool bUseAsyncCook = World && World->IsGameWorld() && bUseAsyncCooking;

	if (bUseAsyncCook)
	{
		AsyncBodySetupQueue.Add(CreateBodySetupHelper());
	}
	else
	{
		AsyncBodySetupQueue.Empty();	//If for some reason we modified the async at runtime, just clear any pending async body setups
		CreateProcMeshBodySetup();
	}

	UBodySetup* UseBodySetup = bUseAsyncCook ? AsyncBodySetupQueue.Last() : ProcMeshBodySetup;

	// Fill in simple collision convex elements
	UseBodySetup->AggGeom.ConvexElems = CollisionConvexElems;

	// Set trace flag
	UseBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	if (bUseAsyncCook)
	{
		UseBodySetup->CreatePhysicsMeshesAsync(FOnAsyncPhysicsCookFinished::CreateUObject(this, &UVoxelProceduralMeshComponent::FinishPhysicsAsyncCook, UseBodySetup));
	}
	else
	{
		// New GUID as collision has changed
		UseBodySetup->BodySetupGuid = FGuid::NewGuid();
		// Also we want cooked data for this
		UseBodySetup->bHasCookedCollisionData = true;
		UseBodySetup->InvalidatePhysicsData();
		UseBodySetup->CreatePhysicsMeshes();
		RecreatePhysicsState();
	}
}

void UVoxelProceduralMeshComponent::FinishPhysicsAsyncCook(UBodySetup* FinishedBodySetup)
{
	TArray<UBodySetup*> NewQueue;
	NewQueue.Reserve(AsyncBodySetupQueue.Num());

	int32 FoundIdx;
	if (AsyncBodySetupQueue.Find(FinishedBodySetup, FoundIdx))
	{
		//The new body was found in the array meaning it's newer so use it
		ProcMeshBodySetup = FinishedBodySetup;
		RecreatePhysicsState();

		//remove any async body setups that were requested before this one
		for (int32 AsyncIdx = FoundIdx + 1; AsyncIdx < AsyncBodySetupQueue.Num(); ++AsyncIdx)
		{
			NewQueue.Add(AsyncBodySetupQueue[AsyncIdx]);
		}

		AsyncBodySetupQueue = NewQueue;
	}
}

UBodySetup* UVoxelProceduralMeshComponent::GetBodySetup()
{
	CreateProcMeshBodySetup();
	return ProcMeshBodySetup;
}

UMaterialInterface* UVoxelProceduralMeshComponent::GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& SectionIndex) const
{
	UMaterialInterface* Result = nullptr;
	SectionIndex = 0;

	// Look for element that corresponds to the supplied face
	int32 TotalFaceCount = 0;
	for (int32 SectionIdx = 0; SectionIdx < ProcMeshSections.Num(); SectionIdx++)
	{
		const FVoxelProcMeshSection& Section = ProcMeshSections[SectionIdx];
		int32 NumFaces = Section.ProcIndexBuffer.Num() / 3;
		TotalFaceCount += NumFaces;

		if (FaceIndex < TotalFaceCount)
		{
			// Grab the material
			Result = GetMaterial(SectionIdx);
			SectionIndex = SectionIdx;
			break;
		}
	}

	return Result;
}