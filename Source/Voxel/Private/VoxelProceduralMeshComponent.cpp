// Copyright 2018 Phyronnaz

#include "VoxelProceduralMeshComponent.h"
#include "VoxelPrivate.h"
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
#if !PLATFORM_ANDROID
#include "ThirdParty/nvtesslib/inc/nvtess.h"
#endif // !PLATFORM_ANDROID
#include "PositionVertexBuffer.h"
#include "StaticMeshVertexBuffer.h"
#include "TessellationRendering.h"
#include "ColorVertexBuffer.h"

DECLARE_STATS_GROUP(TEXT("ProceduralMesh"), STATGROUP_ProceduralMesh, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Create ProcMesh Proxy"), STAT_ProcMesh_CreateSceneProxy, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Create Mesh Section"), STAT_ProcMesh_CreateMeshSection, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("UpdateSection GT"), STAT_ProcMesh_UpdateSectionGT, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("UpdateSection RT"), STAT_ProcMesh_UpdateSectionRT, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Get ProcMesh Elements"), STAT_ProcMesh_GetMeshElements, STATGROUP_ProceduralMesh);
DECLARE_CYCLE_STAT(TEXT("Update Collision"), STAT_ProcMesh_UpdateCollision, STATGROUP_ProceduralMesh);

#if !PLATFORM_ANDROID
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
	FStaticMeshNvRenderBuffer(const FVoxelProcMeshVertexBuffer& InVertexBuffer, const TArray<uint32>& Indices)
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
	const FVoxelProcMeshVertexBuffer& VertexBuffer;

	/** Copying is forbidden. */
	FStaticMeshNvRenderBuffer(const FStaticMeshNvRenderBuffer&);
	FStaticMeshNvRenderBuffer& operator=(const FStaticMeshNvRenderBuffer&);
};


//////////////////////////////////////////////////////////////////////////



static void BuildStaticAdjacencyIndexBuffer(const FVoxelProcMeshVertexBuffer& PositionVertexBuffer, const TArray<uint32>& Indices, TArray<int32>& OutPnAenIndices)
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
#endif // !PLATFORM_ANDROID


//////////////////////////////////////////////////////////////////////////


static void ConvertProcMeshToDynMeshVertex(FVoxelDynamicMeshVertex& Vert, const FVoxelProcMeshVertex& ProcVert)
{
	Vert.Position = ProcVert.Position;
	Vert.Color = ProcVert.Color;
	Vert.TextureCoordinate = ProcVert.TextureCoordinate;
	Vert.TangentX = ProcVert.Tangent.TangentX;
	Vert.TangentZ = ProcVert.Normal;
	Vert.TangentZ.Vector.W = ProcVert.Tangent.bFlipTangentY ? 0 : 255;
}


//////////////////////////////////////////////////////////////////////////



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
		if (SrcSection.ProcIndexBuffer.Num() > 0 && SrcSection.ProcVertexBuffer.Num() > 0)
		{
#if ENGINE_MINOR_VERSION < 19
			FVoxelProcMeshProxySection* NewSection = new FVoxelProcMeshProxySection();
#else
			FVoxelProcMeshProxySection* NewSection = new FVoxelProcMeshProxySection(GetScene().GetFeatureLevel());
#endif

			// Copy data from vertex buffer
			const int32 NumVerts = SrcSection.ProcVertexBuffer.Num();

			// Allocate verts
			NewSection->VertexBuffer.Vertices.SetNumUninitialized(NumVerts);
			// Copy verts
			for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
			{
				const FVoxelProcMeshVertex& ProcVert = SrcSection.ProcVertexBuffer[VertIdx];
				FVoxelDynamicMeshVertex& Vert = NewSection->VertexBuffer.Vertices[VertIdx];
				ConvertProcMeshToDynMeshVertex(Vert, ProcVert);
			}

			// Copy index buffer
			NewSection->IndexBuffer.Indices = SrcSection.ProcIndexBuffer;

			// Init vertex factory
			NewSection->VertexFactory.Init(&NewSection->VertexBuffer);
			
			// Grab material
			NewSection->Material = Component->GetMaterial(SectionIdx);
			if (NewSection->Material == NULL)
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			NewSection->bRequiresAdjacencyInformation = RequiresAdjacencyInformation(NewSection->Material, NewSection->VertexFactory.GetType(), GetScene().GetFeatureLevel());

#if !PLATFORM_ANDROID
			if (NewSection->bRequiresAdjacencyInformation)
			{
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
			}
#endif // !PLATFORM_ANDROID


			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->VertexBuffer);
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->VertexFactory);
			if (NewSection->bRequiresAdjacencyInformation)
			{
				BeginInitResource(&NewSection->AdjacencyIndexBuffer);
			}

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
			Section->VertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();
			if (Section->bRequiresAdjacencyInformation)
			{
				Section->AdjacencyIndexBuffer.ReleaseResource();
			}
			delete Section;
		}
	}
}

void FVoxelProceduralMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView *>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	SCOPE_CYCLE_COUNTER(STAT_ProcMesh_GetMeshElements);
	
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
					BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->VertexBuffer.Vertices.Num() - 1;

					if (Section->IndexBuffer.Indices.Num() != 0 && Section->bRequiresAdjacencyInformation)
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

uint32 FVoxelProceduralMeshSceneProxy::GetAllocatedSize(void) const
{
	return(FPrimitiveSceneProxy::GetAllocatedSize());
}


//////////////////////////////////////////////////////////////////////////


UVoxelProceduralMeshComponent::UVoxelProceduralMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseComplexAsSimpleCollision = true;

	// TODO: This is a fix to crash when selecting component in editor, however VoxelPart won't work with it
	BodyInstance.SetMassOverride(100, true);
}

void UVoxelProceduralMeshComponent::PostLoad()
{
	Super::PostLoad();

	if (ProcMeshBodySetup && IsTemplate())
	{
		ProcMeshBodySetup->SetFlags(RF_Public);
	}
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

	return new FVoxelProceduralMeshSceneProxy(this);
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
	if (GetOwner() && GetOwner()->GetWorld() && GetOwner()->GetWorld()->WorldType != EWorldType::Editor) UpdateCollision(); // Mark collision as dirty
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