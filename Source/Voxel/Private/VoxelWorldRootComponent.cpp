// Copyright 2021 Phyronnaz

#include "VoxelWorldRootComponent.h"
#include "VoxelMinimal.h"
#include "VoxelWorld.h"

#include "PrimitiveSceneProxy.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"

static TAutoConsoleVariable<int32> CVarShowWireframeCollision(
	TEXT("voxel.collision.DrawWireframe"),
	0,
	TEXT("If true, will show the collision as wireframe in the player collision view"),
	ECVF_Default);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelWorldRootComponent::UVoxelWorldRootComponent()
{
	// Tricky: the voxel world will hack to skip registering components when they have bAllowReregistration = false
	// Else it's laggy as hell when editing voxel world properties (too many components)
	// And only registering this one & not the proc meshes messes up the physics
	bAllowReregistration = false;
}

UVoxelWorldRootComponent::~UVoxelWorldRootComponent()
{
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	ensure(ProcMeshesSimpleCollision.Num() == 0);
#endif
}

UBodySetup* UVoxelWorldRootComponent::GetBodySetup()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (!BodySetup)
	{
		BodySetup = NewObject<UBodySetup>(this);
		BodySetup->bGenerateMirroredCollision = false;
	}
	BodySetup->CollisionTraceFlag = CollisionTraceFlag;
	return BodySetup;
}

FBoxSphereBounds UVoxelWorldRootComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return LocalBounds.TransformBy(LocalToWorld);
}

TArray<URuntimeVirtualTexture*> const& UVoxelWorldRootComponent::GetRuntimeVirtualTextures() const
{
	// Disable RVT support on that component
	static TArray<URuntimeVirtualTexture*> Textures;
	return Textures;
}

void UVoxelWorldRootComponent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
	
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	ProcMeshesSimpleCollision.Reset();
#endif
}

void UVoxelWorldRootComponent::TickWorldRoot()
{
	VOXEL_FUNCTION_COUNTER();
	
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	if (bRebuildQueued)
	{
		bRebuildQueued = false;
		RebuildConvexCollision();
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelWorldRootComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	auto* VoxelWorld = Cast<AVoxelWorld>(GetOwner());
	if (!VoxelWorld || 
		!VoxelWorld->IsCreated() || 
		!PropertyChangedEvent.Property)
	{
		return;
	}

	const FName PropertyName = PropertyChangedEvent.Property->GetFName();

	static TArray<FName> RecreateRenderProperties =
	{
		GET_MEMBER_NAME_CHECKED(UVoxelWorldRootComponent, RuntimeVirtualTextures),
		GET_MEMBER_NAME_CHECKED(UVoxelWorldRootComponent, VirtualTextureLodBias),
		GET_MEMBER_NAME_CHECKED(UVoxelWorldRootComponent, VirtualTextureCullMips),
		GET_MEMBER_NAME_CHECKED(UVoxelWorldRootComponent, VirtualTextureMinCoverage),
		GET_MEMBER_NAME_CHECKED(UVoxelWorldRootComponent, VirtualTextureRenderPassType),
	};

	if (RecreateRenderProperties.Contains(PropertyName))
	{
		VoxelWorld->RecreateRender();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
void UVoxelWorldRootComponent::UpdateSimpleCollision(FVoxelProcMeshComponentId Id, FVoxelSimpleCollisionData&& SimpleCollision)
{
	ensure(CollisionTraceFlag != CTF_UseComplexAsSimple);

	if (SimpleCollision.BoxElems.Num() == 0 && SimpleCollision.ConvexElems.Num() == 0)
	{
		if (ProcMeshesSimpleCollision.Remove(Id) == 1)
		{
			bRebuildQueued = true;
		}
		return;
	}

	ProcMeshesSimpleCollision.Add(Id, MakeUnique<FVoxelSimpleCollisionData>(MoveTemp(SimpleCollision)));
	bRebuildQueued = true;
}

void UVoxelWorldRootComponent::RebuildConvexCollision()
{	
	VOXEL_FUNCTION_COUNTER();
	ensure(CollisionTraceFlag != CTF_UseComplexAsSimple);

	{
		VOXEL_SCOPE_COUNTER("Update bounds");
		FBox LocalBox(ForceInit);
		for (auto& It : ProcMeshesSimpleCollision)
		{
			LocalBox += It.Value->Bounds;
		}
		LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(ForceInit); // fallback to reset box sphere bounds
		UpdateBounds();
	}
	
	// Create body setup
	GetBodySetup();

	// Note: we do not need to call ClearPhysicsMeshes, as we are setting the convex meshes manually & handling the ref count ourselves

    TArray<FKBoxElem>& BoxElems = BodySetup->AggGeom.BoxElems;
    TArray<FKConvexElem>& ConvexElems = BodySetup->AggGeom.ConvexElems;

	int32 NumBoxElements = 0;
	int32 NumConvexElements = 0;
    {
        VOXEL_SCOPE_COUNTER("Count");
        for (auto& It : ProcMeshesSimpleCollision)
        {
            const FVoxelSimpleCollisionData& Data = *It.Value;
            NumBoxElements += Data.BoxElems.Num();
            NumConvexElements += Data.ConvexElems.Num();
        }
    }

    if (NumBoxElements == 0 && NumConvexElements == 0) 
    {
        return;
	}

	{
		VOXEL_SCOPE_COUNTER("Lock");
		BodySetupLock.Lock();
	}

	BoxElems.Reset();
	ConvexElems.Reset();
	{
		VOXEL_SCOPE_COUNTER("Reserve");
		BoxElems.Reserve(NumBoxElements);
		ConvexElems.Reserve(NumConvexElements);
	}
	
	{
        VOXEL_SCOPE_COUNTER("Create");
        for (auto& It : ProcMeshesSimpleCollision)
        {
            const FVoxelSimpleCollisionData& Data = *It.Value;

			BoxElems.Append(Data.BoxElems);

            for (int32 Index = 0; Index < Data.ConvexMeshes.Num(); Index++)
            {
                FKConvexElem& NewElement = ConvexElems.Emplace_GetRef();
                // No need to copy the vertices
                NewElement.ElemBox = Data.ConvexElems[Index].ElemBox;
                NewElement.SetConvexMesh(Data.ConvexMeshes[Index].Get());
            }
        }
    }
	{
		VOXEL_SCOPE_COUNTER("Unlock");
		BodySetupLock.Unlock();
	}
	
	{
		// Must not be locked!
		VOXEL_SCOPE_COUNTER("FreeRenderInfo");
		BodySetup->AggGeom.FreeRenderInfo();
	}

	BodySetup->bCreatedPhysicsMeshes = true;

	bool bHasVelocity = false;
	FVector Velocity;
	FVector AngularVelocity;
	if (BodyInstance.IsValidBodyInstance())
	{
		bHasVelocity = FPhysicsCommand::ExecuteRead(BodyInstance.ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			Velocity = FPhysicsInterface::GetLinearVelocity_AssumesLocked(Actor);
			AngularVelocity = FPhysicsInterface::GetAngularVelocity_AssumesLocked(Actor);
		});
		BodyInstance.TermBody();
	}
	
	BodyInstance.InitBody(BodySetup, GetComponentTransform(), this, GetWorld()->GetPhysicsScene());
	
	if (bHasVelocity)
	{
		// Restore velocity
		FPhysicsCommand::ExecuteWrite(BodyInstance.ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			FPhysicsInterface::SetLinearVelocity_AssumesLocked(Actor, Velocity);
			FPhysicsInterface::SetAngularVelocity_AssumesLocked(Actor, AngularVelocity);
		});
	}
}

class UMRMeshComponent
{
public:
	static void FinishCreatingPhysicsMeshes(UBodySetup* Body, const TArray<physx::PxConvexMesh*>& ConvexMeshes, const TArray<physx::PxConvexMesh*>& ConvexMeshesNegX, const TArray<physx::PxTriangleMesh*>& TriMeshes)
	{
		Body->FinishCreatingPhysicsMeshes_PhysX(ConvexMeshes, ConvexMeshesNegX, TriMeshes);
	}
};

void UVoxelWorldRootComponent::SetCookedTriMeshes(const TArray<physx::PxTriangleMesh*>& TriMeshes)
{
	VOXEL_FUNCTION_COUNTER();

	// Create body setup
	GetBodySetup();

	UMRMeshComponent::FinishCreatingPhysicsMeshes(BodySetup, {}, {}, TriMeshes);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FVoxelRenderSimpleCollisionSceneProxy : public FPrimitiveSceneProxy
{
public:
	FVoxelRenderSimpleCollisionSceneProxy(UVoxelWorldRootComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, Component(Component)
		, CollisionResponse(Component->GetCollisionResponseToChannels())
		, CollisionTraceFlag(Component->CollisionTraceFlag)
	{
	}

	//~ Begin FPrimitiveSceneProxy Interface
	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override
	{
		VOXEL_RENDER_FUNCTION_COUNTER();

		// Render bounds
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
			}
		}

		if (!DrawSimpleCollision(ViewFamily.EngineShowFlags)) return;

		const UBodySetup* BodySetup = Component->BodySetup; // Not entirely thread safe, but it's for debug so w/e
		if (!BodySetup) return;
		
		// Catch this here or otherwise GeomTransform below will assert
		if (FMath::Abs(GetLocalToWorld().Determinant()) < SMALL_NUMBER) return;

		const FColor SimpleCollisionColor = FColor(157, 149, 223, 255);

		// Make a material for drawing solid collision stuff
		auto* SolidMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),
			SimpleCollisionColor);
		Collector.RegisterOneFrameMaterialProxy(SolidMaterialInstance);

		const FTransform GeomTransform(GetLocalToWorld());

		const bool bDrawSolid = CVarShowWireframeCollision.GetValueOnRenderThread() == 0;

		FScopeLock Lock(&Component->BodySetupLock);
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (!(VisibilityMap & (1 << ViewIndex))) continue;
			BodySetup->AggGeom.GetAggGeom(
				GeomTransform,
				SimpleCollisionColor,
				SolidMaterialInstance,
				true,
				bDrawSolid,
				DrawsVelocity(),
				ViewIndex,
				Collector);
		}

#if 0
		// Draw wireframe to have one color per hull
		auto* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial->GetRenderProxy(),
			SimpleCollisionColor);
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (!(VisibilityMap & (1 << ViewIndex))) continue;
			BodySetup->AggGeom.GetAggGeom(
				GeomTransform,
				SimpleCollisionColor,
				WireframeMaterialInstance,
				true,
				false,
				DrawsVelocity(),
				ViewIndex,
				Collector);
		}
#endif
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = true;
		Result.bShadowRelevance = false;
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = true;
		Result.bUsesLightingChannels = false;
		Result.bRenderCustomDepth = false;
		Result.bTranslucentSelfShadow = false;
		Result.bVelocityRelevance = false;
		return Result;
	}
	virtual bool CanBeOccluded() const override
	{
		return false;
	}
	virtual uint32 GetMemoryFootprint() const override
	{
		return sizeof(*this);
	}
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

private:
	UVoxelWorldRootComponent* const Component;
	const FCollisionResponseContainer CollisionResponse;
	const ECollisionTraceFlag CollisionTraceFlag;

	bool DrawSimpleCollision(const FEngineShowFlags& EngineShowFlags) const
	{
		const bool bInCollisionView = EngineShowFlags.CollisionVisibility || EngineShowFlags.CollisionPawn;

		// If in a 'collision view' and collision is enabled
		if (bInCollisionView && IsCollisionEnabled())
		{
			// See if we have a response to the interested channel
			bool bHasResponse = EngineShowFlags.CollisionPawn && CollisionResponse.GetResponse(ECC_Pawn) != ECR_Ignore;
			bHasResponse |= EngineShowFlags.CollisionVisibility && CollisionResponse.GetResponse(ECC_Visibility) != ECR_Ignore;

			if (bHasResponse)
			{
				return
					(EngineShowFlags.CollisionPawn && CollisionTraceFlag != ECollisionTraceFlag::CTF_UseComplexAsSimple) ||
					(EngineShowFlags.CollisionVisibility && CollisionTraceFlag == ECollisionTraceFlag::CTF_UseSimpleAsComplex);
			}
		}

		return false;
	}
};

FPrimitiveSceneProxy* UVoxelWorldRootComponent::CreateSceneProxy()
{
#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
	return nullptr;
#else
	return new FVoxelRenderSimpleCollisionSceneProxy(this);
#endif
}