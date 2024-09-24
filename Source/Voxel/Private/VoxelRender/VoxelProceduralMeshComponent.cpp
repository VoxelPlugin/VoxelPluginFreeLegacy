// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProceduralMeshSceneProxy.h"
#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelToolRendering.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/IVoxelProceduralMeshComponent_PhysicsCallbackHandler.h"
#include "VoxelDebug/VoxelDebugManager.h"
#include "VoxelWorldRootComponent.h"
#include "VoxelMessages.h"
#include "VoxelMinimal.h"
#include "IVoxelPool.h"

#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsEngine/BodySetup.h"
#include "AI/NavigationSystemHelpers.h"
#include "AI/NavigationSystemBase.h"
#include "Async/Async.h"
#include "DrawDebugHelpers.h"
#include "Materials/Material.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelPhysicsTriangleMeshesMemory);

static TAutoConsoleVariable<int32> CVarShowCollisionsUpdates(
	TEXT("voxel.renderer.ShowCollisionsUpdates"),
	0,
	TEXT("If true, will show the chunks that finished updating collisions"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IVoxelProceduralMeshComponent_PhysicsCallbackHandler::TickHandler()
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());

	FCallback Callback;
	while (Queue.Dequeue(Callback))
	{
		if (Callback.Component.IsValid())
		{
			Callback.Component->PhysicsCookerCallback(Callback.CookerId);
		}
	}
}

void IVoxelProceduralMeshComponent_PhysicsCallbackHandler::CookerCallback(uint64 CookerId, TWeakObjectPtr<UVoxelProceduralMeshComponent> Component)
{
	Queue.Enqueue({ CookerId, Component });
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::Init(
	int32 InDebugLOD,
	uint32 InDebugChunkId,
	const FVoxelPriorityHandler& InPriorityHandler,
	const TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler>& InPhysicsCallbackHandler,
	const FVoxelRendererSettings& RendererSettings)
{
	ensure(InPhysicsCallbackHandler.IsValid());
	
	bInit = true;
	UniqueId = UNIQUE_ID();
	LOD = InDebugLOD;
	DebugChunkId = InDebugChunkId;
	PriorityHandler = InPriorityHandler;
	PhysicsCallbackHandler = InPhysicsCallbackHandler;
	Pool = RendererSettings.Pool;
	ToolRenderingManager = RendererSettings.ToolRenderingManager;
	PriorityDuration = RendererSettings.PriorityDuration;
	CollisionTraceFlag = RendererSettings.CollisionTraceFlag;
	NumConvexHullsPerAxis = RendererSettings.NumConvexHullsPerAxis;
	bCleanCollisionMesh = RendererSettings.bCleanCollisionMeshes;
	bClearProcMeshBuffersOnFinishUpdate = RendererSettings.bStaticWorld && !RendererSettings.bRenderWorld; // We still need the buffers if we are rendering!
	DistanceFieldSelfShadowBias = RendererSettings.DistanceFieldSelfShadowBias;
	RendererSettings.PrimitiveSettings.ApplyToComponent(*this);
}

void UVoxelProceduralMeshComponent::ClearInit()
{
	ensure(ProcMeshSections.Num() == 0);
	bInit = false;
}

UVoxelProceduralMeshComponent::UVoxelProceduralMeshComponent()
{
	Mobility = EComponentMobility::Movable;
	
	CastShadow = true;
	bUseAsOccluder = true;
	bCanEverAffectNavigation = true;
	
	bAllowReregistration = false; // Slows down the editor when editing properties
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

	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPhysicsTriangleMeshesMemory, MemoryUsage.TriangleMeshes);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelProceduralMeshComponent::AreVoxelCollisionsFrozen()
{
	return bAreCollisionsFrozen;
}

void UVoxelProceduralMeshComponent::SetVoxelCollisionsFrozen(bool bFrozen)
{
	VOXEL_FUNCTION_COUNTER();
	if (bFrozen != bAreCollisionsFrozen)
	{
		if (bFrozen)
		{
			bAreCollisionsFrozen = true;

			OnFreezeVoxelCollisionChanged.Broadcast(true);
		}
		else
		{
			bAreCollisionsFrozen = false;

			for (auto& Component : PendingCollisions)
			{
				if (Component.IsValid())
				{
					Component->UpdateCollision();
				}
			}
			PendingCollisions.Reset();

			OnFreezeVoxelCollisionChanged.Broadcast(false);
		}
	}
}

void UVoxelProceduralMeshComponent::AddOnFreezeVoxelCollisionChanged(const FOnFreezeVoxelCollisionChanged::FDelegate& NewDelegate)
{
	OnFreezeVoxelCollisionChanged.Add(NewDelegate);
}

bool UVoxelProceduralMeshComponent::bAreCollisionsFrozen = false;
TSet<TWeakObjectPtr<UVoxelProceduralMeshComponent>> UVoxelProceduralMeshComponent::PendingCollisions;
FOnFreezeVoxelCollisionChanged UVoxelProceduralMeshComponent::OnFreezeVoxelCollisionChanged;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::SetDistanceFieldData(const TVoxelSharedPtr<const FDistanceFieldVolumeData>& InDistanceFieldData)
{
	if (DistanceFieldData == InDistanceFieldData)
	{
		return;
	}

	DistanceFieldData = InDistanceFieldData;

	GetScene()->UpdatePrimitiveDistanceFieldSceneData_GameThread(this);
	MarkRenderStateDirty();
}

void UVoxelProceduralMeshComponent::SetProcMeshSection(int32 Index, FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update)
{
	VOXEL_FUNCTION_COUNTER();
	if (!ensure(ProcMeshSections.IsValidIndex(Index)))
	{
		return;
	}

	Buffers->UpdateStats();

	ProcMeshSections[Index].Settings = Settings;

	// Due to InitResources etc, we must make sure we are the only component using this buffers, hence the TUniquePtr
	// However the buffer is shared between the component and the proxy
	ProcMeshSections[Index].Buffers = MakeShareable(Buffers.Release());

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

int32 UVoxelProceduralMeshComponent::AddProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update)
{
	VOXEL_FUNCTION_COUNTER();
	check(Buffers.IsValid());

	ensure(Settings.bSectionVisible || Settings.bEnableCollisions || Settings.bEnableNavmesh);
	
	if (Buffers->GetNumIndices() == 0)
	{
		return -1;
	}

	const int32 Index = ProcMeshSections.Emplace();
	SetProcMeshSection(Index, Settings, MoveTemp(Buffers), Update);

	return Index;
}

void UVoxelProceduralMeshComponent::ReplaceProcMeshSection(FVoxelProcMeshSectionSettings Settings, TUniquePtr<FVoxelProcMeshBuffers> Buffers, EVoxelProcMeshSectionUpdate Update)
{
	VOXEL_FUNCTION_COUNTER();
	check(Buffers.IsValid());

	ensure(Settings.bSectionVisible || Settings.bEnableCollisions || Settings.bEnableNavmesh);
	
	int32 SectionIndex = -1;
	for (int32 Index = 0; Index < ProcMeshSections.Num(); Index++)
	{
		if (ProcMeshSections[Index].Settings == Settings)
		{
			ensure(SectionIndex == -1);
			SectionIndex = Index;
		}
	}
	if (SectionIndex == -1)
	{
		AddProcMeshSection(Settings, MoveTemp(Buffers), Update);
	}
	else
	{
		SetProcMeshSection(SectionIndex, Settings, MoveTemp(Buffers), Update);
	}
}

void UVoxelProceduralMeshComponent::ClearSections(EVoxelProcMeshSectionUpdate Update)
{
	VOXEL_FUNCTION_COUNTER();
	ProcMeshSections.Empty();

	if (Update == EVoxelProcMeshSectionUpdate::UpdateNow)
	{
		FinishSectionsUpdates();
	}
}

void UVoxelProceduralMeshComponent::FinishSectionsUpdates()
{
	VOXEL_FUNCTION_COUNTER();
	
	bool bNeedToComputeCollisions = false;
	bool bNeedToComputeNavigation = false;
	{
		TArray<FGuid> NewGuids;
		TMap<FGuid, FVoxelProcMeshSectionSettings> NewGuidToSettings;
		{
			int32 NumGuids = 0;
			for (auto& Section : ProcMeshSections)
			{
				NumGuids += Section.Buffers->Guids.Num();
			}
			NewGuids.Reserve(NumGuids);
			NewGuidToSettings.Reserve(NumGuids);
		}
		for (auto& Section : ProcMeshSections)
		{
			for (auto& Guid : Section.Buffers->Guids)
			{
				NewGuids.Add(Guid);
				ensure(!NewGuidToSettings.Contains(Guid));
				NewGuidToSettings.Add(Guid, Section.Settings);
			}
		}
		NewGuids.Sort();

		if (ProcMeshSectionsSortedGuids != NewGuids)
		{
			bNeedToComputeCollisions = true;
			bNeedToComputeNavigation = true;
		}
		else
		{
			for (auto& Guid : NewGuids)
			{
				const auto& Old = ProcMeshSectionsGuidToSettings[Guid];
				const auto& New = NewGuidToSettings[Guid];
				bNeedToComputeCollisions |= Old.bEnableCollisions != New.bEnableCollisions;
				bNeedToComputeNavigation |= Old.bEnableNavmesh != New.bEnableNavmesh;
			}
		}

		ProcMeshSectionsSortedGuids = MoveTemp(NewGuids);
		ProcMeshSectionsGuidToSettings = MoveTemp(NewGuidToSettings);
	}

	UpdatePhysicalMaterials();
	UpdateLocalBounds();
	MarkRenderStateDirty();

	if (bNeedToComputeCollisions)
	{
		UpdateCollision();
	}
	if (bNeedToComputeNavigation)
	{
		UpdateNavigation();
	}

	if (bClearProcMeshBuffersOnFinishUpdate)
	{
		ProcMeshSections.Reset();
	}

	LastFinishSectionsUpdatesTime = FPlatformTime::Seconds();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FPrimitiveSceneProxy* UVoxelProceduralMeshComponent::CreateSceneProxy()
{
	// Sometimes called outside of the render thread at EndPlay
	VOXEL_ASYNC_FUNCTION_COUNTER();

	for (auto& Section : ProcMeshSections)
	{
		if (Section.Settings.bSectionVisible || FVoxelDebugManager::ShowCollisionAndNavmeshDebug())
		{
			return new FVoxelProceduralMeshSceneProxy(this);
		}
	}

	if (DistanceFieldData.IsValid())
	{
		return new FVoxelProceduralMeshSceneProxy(this);
	}
	
	return nullptr;
}

UBodySetup* UVoxelProceduralMeshComponent::GetBodySetup()
{
	return BodySetup;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UMaterialInterface* UVoxelProceduralMeshComponent::GetMaterialFromCollisionFaceIndex(int32 FaceIndex, int32& OutSectionIndex) const
{
	// Look for element that corresponds to the supplied face
	int32 TotalFaceCount = 0;
	for (int32 SectionIndex = 0; SectionIndex < ProcMeshSections.Num(); SectionIndex++)
	{
		const FVoxelProcMeshSection& Section = ProcMeshSections[SectionIndex];
		const int32 NumFaces = Section.Buffers->GetNumIndices() / 3;
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

int32 UVoxelProceduralMeshComponent::GetNumMaterials() const
{
	int32 Num = ProcMeshSections.Num();
	const auto ToolRenderingManagerPinned = ToolRenderingManager.Pin();
	if (ToolRenderingManagerPinned.IsValid())
	{
		Num += ToolRenderingManagerPinned->GetToolsMaterials().Num();
	}
	return Num;
}

UMaterialInterface* UVoxelProceduralMeshComponent::GetMaterial(int32 Index) const
{
	if (!ensure(Index >= 0)) return nullptr;

	if (Index < ProcMeshSections.Num())
	{
		auto& MaterialPtr = ProcMeshSections[Index].Settings.Material;
		if (MaterialPtr.IsValid())
		{
			return MaterialPtr->GetMaterial();
		}
		else
		{
			return UPrimitiveComponent::GetMaterial(Index);
		}
	}
	else
	{
		Index -= ProcMeshSections.Num();
		const auto ToolRenderingManagerPinned = ToolRenderingManager.Pin();
		if (ToolRenderingManagerPinned.IsValid())
		{
			const auto& Materials = ToolRenderingManagerPinned->GetToolsMaterials();
			if (Materials.IsValidIndex(Index) && ensure(Materials[Index].IsValid()))
			{
				return Materials[Index]->GetMaterial();
			}
		}
		return nullptr;
	}
}

void UVoxelProceduralMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	for (auto& Section : ProcMeshSections)
	{
		if (Section.Settings.Material.IsValid())
		{
			OutMaterials.Add(Section.Settings.Material->GetMaterial());
		}
	}

	const auto ToolRenderingManagerPinned = ToolRenderingManager.Pin();
	if (ToolRenderingManagerPinned.IsValid())
	{
		const auto& Materials = ToolRenderingManagerPinned->GetToolsMaterials();
		for (auto& Material : Materials)
		{
			if (ensure(Material.IsValid()))
			{
				OutMaterials.Add(Material->GetMaterial());
			}
		}
	}
}

FMaterialRelevance UVoxelProceduralMeshComponent::GetMaterialRelevance(ERHIFeatureLevel::Type InFeatureLevel) const
{
	FMaterialRelevance Result;
	const auto Apply = [&](auto* MaterialInterface)
	{
		if (MaterialInterface)
		{
			// MaterialInterface will be null in force delete
			Result |= MaterialInterface->GetRelevance_Concurrent(InFeatureLevel);
		}
	};
	for (auto& Section : ProcMeshSections)
	{
		if (Section.Settings.Material.IsValid())
		{
			Apply(Section.Settings.Material->GetMaterial());
		}
		else
		{
			Apply(UMaterial::GetDefaultMaterial(MD_Surface));
		}
	}

	const auto ToolRenderingManagerPinned = ToolRenderingManager.Pin();
	if (ToolRenderingManagerPinned.IsValid())
	{
		const auto& Materials = ToolRenderingManagerPinned->GetToolsMaterials();
		for (auto& Material : Materials)
		{
			if (ensure(Material.IsValid()))
			{
				Apply(Material->GetMaterial());
			}
		}
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool UVoxelProceduralMeshComponent::DoCustomNavigableGeometryExport(FNavigableGeometryExport& GeomExport) const
{
	VOXEL_FUNCTION_COUNTER();

	for (auto& Section : ProcMeshSections)
	{
		if (Section.Settings.bEnableNavmesh)
		{
			TArray<FVector> Vertices;
			// TODO is that copy needed
			{
				auto& PositionBuffer = Section.Buffers->VertexBuffers.PositionVertexBuffer;
				Vertices.SetNumUninitialized(PositionBuffer.GetNumVertices());
				for (int32 Index = 0; Index < Vertices.Num(); Index++)
				{
					Vertices[Index] = FVector(PositionBuffer.VertexPosition(Index));
				}
			}
			TArray<int32> Indices;
			// Copy needed because int32 vs uint32
			{
				auto& IndexBuffer = Section.Buffers->IndexBuffer;
				Indices.SetNumUninitialized(IndexBuffer.GetNumIndices());
				for (int32 Index = 0; Index < Indices.Num(); Index++)
				{
					Indices[Index] = IndexBuffer.GetIndex(Index);
				}
			}
			GeomExport.ExportCustomMesh(Vertices.GetData(), Vertices.Num(), Indices.GetData(), Indices.Num(), GetComponentTransform());
		}
	}
	return false;
}

FBoxSphereBounds UVoxelProceduralMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds(LocalBounds.TransformBy(LocalToWorld));
}

void UVoxelProceduralMeshComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	UPrimitiveComponent::OnComponentDestroyed(bDestroyingHierarchy);
	
	// Destroy async cooker
	if (AsyncCooker)
	{
		AsyncCooker->CancelAndAutodelete();
		AsyncCooker = nullptr;
	}
	
	// Clear memory
	ProcMeshSections.Reset();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::UpdatePhysicalMaterials()
{
	VOXEL_FUNCTION_COUNTER();

	FBodyInstance* BodyInst = GetBodyInstance();
	if (BodyInst && BodyInst->IsValidBodyInstance())
	{
		BodyInst->UpdatePhysicalMaterials();
	}
}

void UVoxelProceduralMeshComponent::UpdateLocalBounds()
{
	VOXEL_FUNCTION_COUNTER();
	
	FBox LocalBox(ForceInit);

	for (auto& Section : ProcMeshSections)
	{
		LocalBox += Section.Buffers->LocalBounds;
	}

	LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(ForceInit); // fallback to reset box sphere bounds

	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}

void UVoxelProceduralMeshComponent::UpdateNavigation()
{
	VOXEL_FUNCTION_COUNTER();

	if (CanEverAffectNavigation() && IsRegistered() && GetWorld() && GetWorld()->GetNavigationSystem() && FNavigationSystem::WantsComponentChangeNotifies())
	{
		bNavigationRelevant = IsNavigationRelevant();
		FNavigationSystem::UpdateComponentData(*this);
	}
}

void UVoxelProceduralMeshComponent::UpdateCollision()
{
	VOXEL_FUNCTION_COUNTER();

	if (!ensure(GetWorld()))
	{
		return;
	}

	if (bAreCollisionsFrozen)
	{
		PendingCollisions.Add(this);
		return;
	}

	// Cancel existing task
	if (AsyncCooker)
	{
		AsyncCooker->CancelAndAutodelete();
		AsyncCooker = nullptr;
		ensure(BodySetupBeingCooked);
	}

	if (!BodySetupBeingCooked)
	{
		BodySetupBeingCooked = NewObject<UBodySetup>(this);
	}
	VOXEL_INLINE_COUNTER("ClearPhysicsMeshes", BodySetupBeingCooked->ClearPhysicsMeshes());
	BodySetupBeingCooked->bGenerateMirroredCollision = false;
	BodySetupBeingCooked->CollisionTraceFlag = CollisionTraceFlag;

	if (ProcMeshSections.FindByPredicate([](auto& Section) { return Section.Settings.bEnableCollisions; }))
	{
		auto PoolPtr = Pool.Pin();
		if (ensure(PoolPtr.IsValid()))
		{
			AsyncCooker = IVoxelAsyncPhysicsCooker::CreateCooker(this);
			if (ensure(AsyncCooker))
			{
				PoolPtr->QueueTask(EVoxelTaskType::CollisionCooking, AsyncCooker);
			}
		}
	}
	else
	{
		FinishCollisionUpdate();
	}
}

void UVoxelProceduralMeshComponent::FinishCollisionUpdate()
{
	VOXEL_FUNCTION_COUNTER();
	
	ensure(BodySetupBeingCooked);

	Swap(BodySetup, BodySetupBeingCooked);
	RecreatePhysicsState();

	if (BodySetupBeingCooked)
	{
		BodySetupBeingCooked->ClearPhysicsMeshes();
	}

	if (CVarShowCollisionsUpdates.GetValueOnGameThread() &&
		ProcMeshSections.FindByPredicate([](auto& Section) { return Section.Settings.bEnableCollisions; }))
	{
		const auto Box = Bounds.GetBox();
		DrawDebugBox(GetWorld(), Box.GetCenter(), Box.GetExtent(), FColor::Red, false, 0.1);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelProceduralMeshComponent::PhysicsCookerCallback(uint64 CookerId)
{
	VOXEL_FUNCTION_COUNTER();
	check(IsInGameThread());
	
	if (!AsyncCooker || CookerId != AsyncCooker->UniqueId)
	{
		LOG_VOXEL(VeryVerbose, TEXT("Late async cooker callback, ignoring it"));
		return;
	}
	if (!ensure(AsyncCooker->IsDone()) || !ensure(BodySetupBeingCooked))
	{
		return;
	}

	// Might not be needed?
	VOXEL_INLINE_COUNTER("ClearPhysicsMeshes", BodySetupBeingCooked->ClearPhysicsMeshes());

	FVoxelProceduralMeshComponentMemoryUsage NewMemoryUsage;
	if (!AsyncCooker->Finalize(*BodySetupBeingCooked, NewMemoryUsage))
	{
		return;
	}
	
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPhysicsTriangleMeshesMemory, MemoryUsage.TriangleMeshes);
	MemoryUsage.TriangleMeshes = NewMemoryUsage.TriangleMeshes;
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelPhysicsTriangleMeshesMemory, MemoryUsage.TriangleMeshes);

	AsyncCooker->CancelAndAutodelete();
	AsyncCooker = nullptr;
	
	FinishCollisionUpdate();
}