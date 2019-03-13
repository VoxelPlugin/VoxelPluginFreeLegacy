// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProceduralMeshSceneProxy.h"
#include "VoxelRender/VoxelAsyncPhysicsCooker.h"
#include "VoxelGlobals.h"
#include "IVoxelPool.h"

#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "AI/NavigationSystemHelpers.h"
#include "AI/NavigationSystemBase.h"
#include "Async/Async.h"
#include "DrawDebugHelpers.h"

DECLARE_CYCLE_STAT(TEXT("UVoxelProceduralMeshComponent::CreateSceneProxy")               , STAT_UVoxelProceduralMeshComponent_CreateSceneProxy               , STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProceduralMeshComponent::UpdateCollision")                , STAT_UVoxelProceduralMeshComponent_UpdateCollision                , STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProceduralMeshComponent::UpdateNavigation")               , STAT_UVoxelProceduralMeshComponent_UpdateNavigation               , STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProceduralMeshComponent::AsyncPhysicsCookerCallback")     , STAT_UVoxelProceduralMeshComponent_AsyncPhysicsCookerCallback     , STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("UVoxelProceduralMeshComponent::DoCustomNavigableGeometryExport"), STAT_UVoxelProceduralMeshComponent_DoCustomNavigableGeometryExport, STATGROUP_Voxel);

static TAutoConsoleVariable<int32> CVarShowCollisionsUpdates(
	TEXT("voxel.ShowCollisionsUpdates"),
	0,
	TEXT("If true, will show the chunks that finished updating collisions"),
	ECVF_Default);

void FVoxelProcMeshSection::Check()
{
#if DO_CHECK
	for (int Index : Indices)
	{
		check(0 <= Index && Index < Positions.Num());
	}
	for (int Index : AdjacencyIndices)
	{
		check(0 <= Index && Index < Positions.Num());
	}

	check(Positions.Num() == Normals.Num() &&
		Positions.Num() == Tangents.Num() &&
		Positions.Num() == Colors.Num() &&
		Positions.Num() == TextureCoordinates.Num());
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelProceduralMeshComponent::UVoxelProceduralMeshComponent()
{
	bCastShadowAsTwoSided = true;
	bHasCustomNavigableGeometry = EHasCustomNavigableGeometry::EvenIfNotCollidable;

	// Fix for details crash
	BodyInstance.SetMassOverride(100, true);
}

UVoxelProceduralMeshComponent::~UVoxelProceduralMeshComponent()
{
	if (AsyncCooker)
	{
		AsyncCooker->CancelAndAutodelete();
		AsyncCooker = nullptr;
	}
}

void UVoxelProceduralMeshComponent::SetCollisionConvexMeshes(TArray<FKConvexElem>&& InCollisionConvexElems)
{
	CollisionConvexElems = InCollisionConvexElems;
	UpdateCollision();
}

void UVoxelProceduralMeshComponent::SetProcMeshSection(int32 SectionIndex, FVoxelProcMeshSection&& Section, EVoxelProcMeshSectionUpdate Update)
{
	// Ensure sections array is long enough
	if (SectionIndex >= ProcMeshSections.Num())
	{
		ProcMeshSections.SetNum(SectionIndex + 1);
	}

	ProcMeshSections[SectionIndex] = MoveTemp(Section);

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

void UVoxelProceduralMeshComponent::ClearSections(EVoxelProcMeshSectionUpdate Update)
{
	ProcMeshSections.Empty();

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

void UVoxelProceduralMeshComponent::FinishSectionsUpdates()
{
	UpdateLocalBounds(); // Update overall bounds
	UpdateNavigation(); // Update navigation
	UpdateCollision(); // Mark collision as dirty
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

void UVoxelProceduralMeshComponent::UpdatePhysicalMaterials()
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

bool UVoxelProceduralMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	int32 VertexBase = 0; // Base vertex index for current section

	// For each section..
	for (int32 SectionIdx = 0; SectionIdx < ProcMeshSections.Num(); SectionIdx++)
	{
		FVoxelProcMeshSection& Section = ProcMeshSections[SectionIdx];
		// Do we have collision enabled?
		if (Section.bEnableCollision)
		{
			// Copy vertices
			CollisionData->Vertices.Append(Section.Positions);

			// Copy triangle data
			for (int32 Index = 0; Index < Section.Indices.Num(); Index += 3)
			{
				// Need to add base offset for indices
				FTriIndices Indices;
				Indices.v0 = Section.Indices[Index + 0] + VertexBase;
				Indices.v1 = Section.Indices[Index + 1] + VertexBase;
				Indices.v2 = Section.Indices[Index + 2] + VertexBase;
				CollisionData->Indices.Emplace(Indices);

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

FPrimitiveSceneProxy* UVoxelProceduralMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProceduralMeshComponent_CreateSceneProxy);

	for (auto& Section : ProcMeshSections)
	{
		if (Section.bSectionVisible)
		{
			return new FVoxelProceduralMeshSceneProxy(this);
		}
	}
	return nullptr;
}

UBodySetup* UVoxelProceduralMeshComponent::GetBodySetup()
{
	CreateProcMeshBodySetup();
	return ProcMeshBodySetup;
}

UMaterialInterface* UVoxelProceduralMeshComponent::GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& OutSectionIndex) const
{
	// Look for element that corresponds to the supplied face
	int32 TotalFaceCount = 0;
	for (int32 SectionIndex = 0; SectionIndex < ProcMeshSections.Num(); SectionIndex++)
	{
		const FVoxelProcMeshSection& Section = ProcMeshSections[SectionIndex];
		int32 NumFaces = Section.Indices.Num() / 3;
		TotalFaceCount += NumFaces;

		if (FaceIndex < TotalFaceCount)
		{
			OutSectionIndex = SectionIndex;
			return GetMaterial(SectionIndex);
		}
	}
	OutSectionIndex = 0;
	return nullptr;
}

void UVoxelProceduralMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	for (int SectionIndex = 0; SectionIndex < ProcMeshSections.Num(); SectionIndex++)
	{
		if (UMaterialInterface* Material = GetMaterial(SectionIndex))
		{
			OutMaterials.Add(Material);
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

bool UVoxelProceduralMeshComponent::DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProceduralMeshComponent_DoCustomNavigableGeometryExport);

	for (auto& Section : ProcMeshSections)
	{
		if (Section.bEnableNavmesh)
		{
			TArray<int32> Indices;
			// int32 vs uint32 so we must iterate
			Indices.SetNumUninitialized(Section.Indices.Num());
			for (int Index = 0; Index < Indices.Num(); Index++)
			{
				Indices[Index] = Section.Indices[Index];
			}
			GeomExport.ExportCustomMesh(Section.Positions.GetData(), Section.Positions.Num(), Indices.GetData(), Indices.Num(), GetComponentTransform());
		}
	}
	return false;
}

FBoxSphereBounds UVoxelProceduralMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));

	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;

	return Ret;
}

void UVoxelProceduralMeshComponent::PostLoad()
{
	Super::PostLoad();

	if (ProcMeshBodySetup && IsTemplate())
	{
		ProcMeshBodySetup->SetFlags(RF_Public);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::UpdateLocalBounds()
{
	FBox LocalBox(ForceInit);

	for (auto& Section : ProcMeshSections)
	{
		LocalBox += Section.SectionLocalBox;
	}

	LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0); // fallback to reset box sphere bounds

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

void UVoxelProceduralMeshComponent::UpdateNavigation()
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProceduralMeshComponent_UpdateNavigation);

	if (CanEverAffectNavigation() && IsRegistered() && GetWorld() && GetWorld()->GetNavigationSystem() && FNavigationSystem::WantsComponentChangeNotifies())
	{
		bNavigationRelevant = IsNavigationRelevant();
		FNavigationSystem::UpdateComponentData(*this);
	}
}

void UVoxelProceduralMeshComponent::UpdateCollision()
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProceduralMeshComponent_UpdateCollision);

	if (!bEnableCollisions || !ensure(GetWorld()))
	{
		return;
	}

	// Cancel exiting task
	if (AsyncCooker)
	{
		AsyncCooker->CancelAndAutodelete();
		AsyncCooker = nullptr;
		ensure(BodySetupBeingCooked);
		BodySetupBeingCooked = nullptr;
	}

	ensure(!BodySetupBeingCooked);

	BodySetupBeingCooked = CreateBodySetupHelper();

	// Fill in simple collision convex elements
	BodySetupBeingCooked ->AggGeom.ConvexElems = CollisionConvexElems;

	// Set trace flag
	BodySetupBeingCooked ->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	AsyncCooker = new FVoxelAsyncPhysicsCooker(this, BodySetupBeingCooked);
	if (AsyncCooker->HasSomethingToCook())
	{
		if (ensure(Pool.IsValid()))
		{
			Pool->QueueCollisionTask(AsyncCooker);
		}
	}
	else
	{
		FinishCollisionUpdate();
	}
}

void UVoxelProceduralMeshComponent::FinishCollisionUpdate()
{
	check(AsyncCooker);
	check(BodySetupBeingCooked);

	ProcMeshBodySetup = BodySetupBeingCooked;
	RecreatePhysicsState();

	AsyncCooker->CancelAndAutodelete();
	AsyncCooker = nullptr;
	BodySetupBeingCooked = nullptr;

	if (CVarShowCollisionsUpdates.GetValueOnGameThread())
	{
		auto Box = Bounds.GetBox();
		DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), FColor::Red, false, 0.1);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::AsyncPhysicsCookerCallback(uint64 CookerId)
{
	SCOPE_CYCLE_COUNTER(STAT_UVoxelProceduralMeshComponent_AsyncPhysicsCookerCallback);

	check(IsInGameThread());

	if (!AsyncCooker || CookerId != AsyncCooker->UniqueId)
	{
		UE_LOG(LogVoxel, VeryVerbose, TEXT("Late async cooker callback, ignoring it"));
		return;
	}
	
	if (!AsyncCooker->IsSuccessful())
	{
		UE_LOG(LogVoxel, Warning, TEXT("Async cooker wasn't successful, ignoring it"));
		return;
	}

	if (!ensure(BodySetupBeingCooked))
	{
		UE_LOG(LogVoxel, Error, TEXT("Invalid BodySetupBeingCooked!"));
		return;
	}

	BodySetupBeingCooked->FinishCreatingPhysicsMeshes(AsyncCooker->OutNonMirroredConvexMeshes, AsyncCooker->OutMirroredConvexMeshes, AsyncCooker->OutTriangleMeshes);
	FinishCollisionUpdate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::CreateProcMeshBodySetup()
{
	if (!ProcMeshBodySetup)
	{
		ProcMeshBodySetup = CreateBodySetupHelper();
	}
}

UBodySetup* UVoxelProceduralMeshComponent::CreateBodySetupHelper()
{
	UBodySetup* NewBodySetup = NewObject<UBodySetup>(this);
	NewBodySetup->bGenerateMirroredCollision = false;
	return NewBodySetup;
}