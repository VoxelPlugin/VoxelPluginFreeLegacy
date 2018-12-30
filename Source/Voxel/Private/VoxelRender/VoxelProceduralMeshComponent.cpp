// Copyright 2018 Phyronnaz

#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelLogStatDefinitions.h"
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
#include "PhysicsEngine/PhysicsSettings.h"
#include "Stats/Stats.h"
#include "Rendering/PositionVertexBuffer.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "TessellationRendering.h"
#include "Rendering/ColorVertexBuffer.h"
#include "Runtime/Launch/Resources/Version.h"
#include "HAL/Platform.h"
#include "VoxelGlobals.h"
#include "AI/NavigationSystemBase.h"
#include "VoxelLogStatDefinitions.h"

DECLARE_CYCLE_STAT(TEXT("Create ProcMesh Proxy"), STAT_VoxelProcMesh_CreateSceneProxy, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("Copy data"), STAT_VoxelProcMesh_CreateSceneProxy_CopyData, STATGROUP_Voxel);

DECLARE_CYCLE_STAT(TEXT("Get ProcMesh Elements"), STAT_VoxelProcMesh_GetMeshElements, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("Update Collision"), STAT_VoxelProcMesh_UpdateCollision, STATGROUP_Voxel);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelRawStaticIndexBuffer::FVoxelRawStaticIndexBuffer(bool InNeedsCPUAccess)
	: IndexStorage(InNeedsCPUAccess)
	, b32Bit(false)
{
}

void FVoxelRawStaticIndexBuffer::SetIndices(const TArray<uint32>& InIndices, EIndexBufferStride::Type DesiredStride)
{
	int32 NumInIndices = InIndices.Num();
	bool bShouldUse32Bit = false;

	// Figure out if we should store the indices as 16 or 32 bit.
	if (DesiredStride == EIndexBufferStride::Force32Bit)
	{
		bShouldUse32Bit = true;
	}
	else if (DesiredStride == EIndexBufferStride::AutoDetect)
	{
		int32 i = 0;
		while (!bShouldUse32Bit && i < NumInIndices)
		{
			bShouldUse32Bit = InIndices[i] > MAX_uint16;
			i++;
		}
	}

	// Allocate storage for the indices.
	int32 IndexStride = bShouldUse32Bit ? sizeof(uint32) : sizeof(uint16);
	IndexStorage.Empty(IndexStride * NumInIndices);
	IndexStorage.AddUninitialized(IndexStride * NumInIndices);

	// Store them!
	if (bShouldUse32Bit)
	{
		// If the indices are 32 bit we can just do a memcpy.
		check(IndexStorage.Num() == InIndices.Num() * InIndices.GetTypeSize());
		FMemory::Memcpy(IndexStorage.GetData(),InIndices.GetData(),IndexStorage.Num());
		b32Bit = true;
	}
	else
	{
		// Copy element by element demoting 32-bit integers to 16-bit.
		check(IndexStorage.Num() == InIndices.Num() * sizeof(uint16));
		uint16* DestIndices16Bit = (uint16*)IndexStorage.GetData();
		for (int32 i = 0; i < NumInIndices; ++i)
		{
			DestIndices16Bit[i] = InIndices[i];
		}
		b32Bit = false;
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::InsertIndices( const uint32 At, const uint32* IndicesToAppend, const uint32 NumIndicesToAppend )
{
	if( NumIndicesToAppend > 0 )
	{
		const uint32 IndexStride = b32Bit ? sizeof( uint32 ) : sizeof( uint16 );

		IndexStorage.InsertUninitialized( At * IndexStride, NumIndicesToAppend * IndexStride );
		uint8* const DestIndices = &IndexStorage[ At * IndexStride ];

		if( IndicesToAppend )
		{
			if( b32Bit )
			{
				// If the indices are 32 bit we can just do a memcpy.
				FMemory::Memcpy( DestIndices, IndicesToAppend, NumIndicesToAppend * IndexStride );
			}
			else
			{
				// Copy element by element demoting 32-bit integers to 16-bit.
				uint16* DestIndices16Bit = (uint16*)DestIndices;
				for( uint32 Index = 0; Index < NumIndicesToAppend; ++Index )
				{
					DestIndices16Bit[ Index ] = IndicesToAppend[ Index ];
				}
			}
		}
		else
		{
			// If no indices to insert were supplied, just clear the buffer
			FMemory::Memset( DestIndices, 0, NumIndicesToAppend * IndexStride );
		}
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::AppendIndices( const uint32* IndicesToAppend, const uint32 NumIndicesToAppend )
{
	InsertIndices( b32Bit ? IndexStorage.Num() / 4 : IndexStorage.Num() / 2, IndicesToAppend, NumIndicesToAppend );
}

void FVoxelRawStaticIndexBuffer::RemoveIndicesAt( const uint32 At, const uint32 NumIndicesToRemove )
{
	if( NumIndicesToRemove > 0 )
	{
		const int32 IndexStride = b32Bit ? sizeof( uint32 ) : sizeof( uint16 );
		IndexStorage.RemoveAt( At * IndexStride, NumIndicesToRemove * IndexStride );
	}

	UpdateCachedNumIndices();
}

void FVoxelRawStaticIndexBuffer::GetCopy(TArray<uint32>& OutIndices) const
{
	OutIndices.Empty(NumIndices);
	OutIndices.AddUninitialized(NumIndices);

	if (b32Bit)
	{
		// If the indices are 32 bit we can just do a memcpy.
		check(IndexStorage.Num() == OutIndices.Num() * OutIndices.GetTypeSize());
		FMemory::Memcpy(OutIndices.GetData(),IndexStorage.GetData(),IndexStorage.Num());
	}
	else
	{
		// Copy element by element promoting 16-bit integers to 32-bit.
		check(IndexStorage.Num() == OutIndices.Num() * sizeof(uint16));
		const uint16* SrcIndices16Bit = (const uint16*)IndexStorage.GetData();
		for (uint32 i = 0; i < NumIndices; ++i)
		{
			OutIndices[i] = SrcIndices16Bit[i];
		}
	}
}

const uint16* FVoxelRawStaticIndexBuffer::AccessStream16() const
{
	if (!b32Bit)
	{
		return reinterpret_cast<const uint16*>(IndexStorage.GetData());
	}
	return nullptr;
}

FIndexArrayView FVoxelRawStaticIndexBuffer::GetArrayView() const
{
	return FIndexArrayView(IndexStorage.GetData(), NumIndices, b32Bit);
}

void FVoxelRawStaticIndexBuffer::InitRHI()
{
	uint32 IndexStride = b32Bit ? sizeof(uint32) : sizeof(uint16);
	uint32 SizeInBytes = IndexStorage.Num();

	if (SizeInBytes > 0)
	{
		// Create the index buffer.
		FRHIResourceCreateInfo CreateInfo(&IndexStorage);
		IndexBufferRHI = RHICreateIndexBuffer(IndexStride,SizeInBytes,BUF_Static,CreateInfo);
	}    
}

void FVoxelRawStaticIndexBuffer::Serialize(FArchive& Ar, bool bNeedsCPUAccess)
{
	IndexStorage.SetAllowCPUAccess(bNeedsCPUAccess);

	if (Ar.UE4Ver() < VER_UE4_SUPPORT_32BIT_STATIC_MESH_INDICES)
	{
		TResourceArray<uint16,INDEXBUFFER_ALIGNMENT> LegacyIndices;

		b32Bit = false;
		LegacyIndices.BulkSerialize(Ar);
		int32 NumLegacyIndices = LegacyIndices.Num();
		int32 IndexStride = sizeof(uint16);
		IndexStorage.Empty(NumLegacyIndices * IndexStride);
		IndexStorage.AddUninitialized(NumLegacyIndices * IndexStride);
		FMemory::Memcpy(IndexStorage.GetData(),LegacyIndices.GetData(),IndexStorage.Num());
	}
	else
	{
		Ar << b32Bit;
		IndexStorage.BulkSerialize(Ar);
	}
}

void FVoxelRawStaticIndexBuffer::Discard()
{
    IndexStorage.SetAllowCPUAccess(false);
    IndexStorage.Discard();

	UpdateCachedNumIndices();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

FVoxelProceduralMeshSceneProxy::FVoxelProceduralMeshSceneProxy(UVoxelProceduralMeshComponent* Component)
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
				SCOPE_CYCLE_COUNTER(STAT_VoxelProcMesh_CreateSceneProxy_CopyData);

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
	SCOPE_CYCLE_COUNTER(STAT_VoxelProcMesh_GetMeshElements);
	
	// Iterate over sections
	for (FVoxelProcMeshProxySection* Section : Sections)
	{
		if (Section != nullptr && Section->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = Section->Material->GetRenderProxy(IsSelected());

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

					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					BatchElement.PrimitiveUniformBufferResource = &GetUniformBuffer();
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.GetNumIndices() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

					if (Section->IndexBuffer.GetNumIndices() != 0 && Section->bRequiresAdjacencyInformation)
					{
						BatchElement.IndexBuffer = &Section->AdjacencyIndexBuffer;
						Mesh.Type = PT_12_ControlPointPatchList;
						BatchElement.FirstIndex *= 4;
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
	return(sizeof(*this) + GetAllocatedSize());
}

uint32 FVoxelProceduralMeshSceneProxy::GetAllocatedSize() const
{
	return(FPrimitiveSceneProxy::GetAllocatedSize());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelProceduralMeshComponent::UVoxelProceduralMeshComponent()
{
	bCastShadowAsTwoSided = true;

	// Fix for details crash
	BodyInstance.SetMassOverride(100, true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
			for (int32 VertIdx = 0; VertIdx < Section.Positions.Num(); VertIdx++)
			{
				CollisionData->Vertices.Add(Section.Positions[VertIdx]);
			}

			// Copy triangle data
			const int32 NumTriangles = Section.Indices.Num() / 3;
			for (int32 TriIdx = 0; TriIdx < NumTriangles; TriIdx++)
			{
				// Need to add base offset for indices
				FTriIndices Triangle;
				Triangle.v0 = Section.Indices[(TriIdx * 3) + 0] + VertexBase;
				Triangle.v1 = Section.Indices[(TriIdx * 3) + 1] + VertexBase;
				Triangle.v2 = Section.Indices[(TriIdx * 3) + 2] + VertexBase;
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
		if (Section.Indices.Num() >= 3 && Section.bEnableCollision)
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelProceduralMeshComponent::GetNumSections() const
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
		SectionMaterials.SetNum(SectionIndex + 1, false);
	}

	ProcMeshSections[SectionIndex] = Section;
	SectionMaterials[SectionIndex] = Section.Material;

	UpdateLocalBounds(); // Update overall bounds
	UpdateCollision(); // Mark collision as dirty
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

void UVoxelProceduralMeshComponent::SetProcMeshSection(int32 SectionIndex, FVoxelProcMeshSection&& Section, EVoxelProcMeshSectionUpdate Update)
{
	// Ensure sections array is long enough
	if (SectionIndex >= ProcMeshSections.Num())
	{
		ProcMeshSections.SetNum(SectionIndex + 1, false);
		SectionMaterials.SetNum(SectionIndex + 1, false);
	}

	ProcMeshSections[SectionIndex] = Section;
	SectionMaterials[SectionIndex] = Section.Material;

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

void UVoxelProceduralMeshComponent::ClearSections(EVoxelProcMeshSectionUpdate Update)
{
	ProcMeshSections.Empty();
	SectionMaterials.Empty();

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

void UVoxelProceduralMeshComponent::FinishSectionsUpdates()
{
	UpdateLocalBounds(); // Update overall bounds
	UpdateCollision(); // Mark collision as dirty
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

void UVoxelProceduralMeshComponent::UpdateMaterials()
{
	FBodyInstance* BodyInst = GetBodyInstance();
	if (BodyInst && BodyInst->IsValidBodyInstance())
	{
		BodyInst->UpdatePhysicalMaterials();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPrimitiveSceneProxy* UVoxelProceduralMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelProcMesh_CreateSceneProxy);

	return new FVoxelProceduralMeshSceneProxy(this);
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
		int32 NumFaces = Section.Indices.Num() / 3;
		TotalFaceCount += NumFaces;

		if (FaceIndex < TotalFaceCount)
		{
			// Grab the material
			Result = SectionMaterials[SectionIdx] ? SectionMaterials[SectionIdx] : GetMaterial(SectionIdx);
			SectionIndex = SectionIdx;
			break;
		}
	}

	return Result;
}

void UVoxelProceduralMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	for (int Index = 0; Index < GetNumSections(); Index++)
	{
		UMaterialInterface* Material = ProcMeshSections[Index].Material;
		if (Material)
		{
			OutMaterials.Add(Material);
		}
		else
		{
			OutMaterials.Add(Super::GetMaterial(Index));
		}
	}
}

UMaterialInterface* UVoxelProceduralMeshComponent::GetMaterial(int32 Index) const
{
	if (ProcMeshSections.IsValidIndex(Index))
	{
		UMaterialInterface* Material = ProcMeshSections[Index].Material;
		if (Material)
		{
			return Material;
		}
		else
		{
			return Super::GetMaterial(Index);
		}
	}
	else
	{
		return nullptr;
	}
}

///////////////////////////////////////////////////////////////////////////////

int32 UVoxelProceduralMeshComponent::GetNumMaterials() const
{
	return ProcMeshSections.Num();
}

///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::PostLoad()
{
	Super::PostLoad();

	if (ProcMeshBodySetup && IsTemplate())
	{
		ProcMeshBodySetup->SetFlags(RF_Public);
	}
}

///////////////////////////////////////////////////////////////////////////////

FBoxSphereBounds UVoxelProceduralMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::UpdateCollision()
{
	SCOPE_CYCLE_COUNTER(STAT_VoxelProcMesh_UpdateCollision);

	UWorld* World = GetWorld();
	const bool bUseAsyncCook = World && bUseAsyncCooking;

	if (bUseAsyncCook)
	{
#if ENGINE_MINOR_VERSION >= 21
		// Abort all previous ones still standing
		for (UBodySetup* OldBody : AsyncBodySetupQueue)
		{
			OldBody->AbortPhysicsMeshAsyncCreation();
		}
#endif

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
		UseBodySetup->CreatePhysicsMeshesAsync(FOnAsyncPhysicsCookFinished::CreateUObject(this, &UVoxelProceduralMeshComponent::FinishPhysicsAsyncCook, 
#if ENGINE_MINOR_VERSION < 21
			true,
#endif
			UseBodySetup));
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

void UVoxelProceduralMeshComponent::CreateProcMeshBodySetup()
{
	if (ProcMeshBodySetup == nullptr)
	{
		ProcMeshBodySetup = CreateBodySetupHelper();
	}
}

void UVoxelProceduralMeshComponent::FinishPhysicsAsyncCook(bool bSuccess, UBodySetup* FinishedBodySetup)
{
	TArray<UBodySetup*> NewQueue;
	NewQueue.Reserve(AsyncBodySetupQueue.Num());

	int32 FoundIdx;
	if (AsyncBodySetupQueue.Find(FinishedBodySetup, FoundIdx))
	{
		if (bSuccess)
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
		else
		{
			AsyncBodySetupQueue.RemoveAt(FoundIdx);
		}

		// Now update the navigation.
		if (FNavigationSystem::WantsComponentChangeNotifies() && IsRegistered())
		{
			UWorld* MyWorld = GetWorld();

			if (MyWorld != nullptr && MyWorld->GetNavigationSystem() != nullptr)
			{
				FNavigationSystem::UpdateComponentData(*this);
			}
		}
	}
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