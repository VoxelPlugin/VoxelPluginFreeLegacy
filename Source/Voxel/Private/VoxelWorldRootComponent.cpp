// Copyright 2020 Phyronnaz

#include "VoxelWorldRootComponent.h"
#include "VoxelMinimal.h"
#include "VoxelWorld.h"

#include "PhysXIncludes.h"
#include "PrimitiveSceneProxy.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
UVoxelWorldRootComponent::FConvexElements::FConvexElements(
	const FBox& Bounds,
	TArray<FKConvexElem>&& InConvexElements,
	TArray<physx::PxConvexMesh*>&& InConvexMeshes)
	: Bounds(Bounds)
	, ConvexElements(MoveTemp(InConvexElements))
	, ConvexMeshes(MoveTemp(InConvexMeshes))
{
	ensure(Bounds.IsValid);
	ensure(ConvexElements.Num() == ConvexMeshes.Num());
	for (auto* ConvexMesh : ConvexMeshes)
	{
		ConvexMesh->acquireReference();
	}
}

UVoxelWorldRootComponent::FConvexElements::~FConvexElements()
{
	for (auto* ConvexMesh : ConvexMeshes)
	{
		ConvexMesh->release();
	}
}
#endif

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
	ensure(Elements.Num() == 0);
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
void UVoxelWorldRootComponent::UpdateConvexCollision(
	uint64 Id,
	const FBox& InBounds,
	TArray<FKConvexElem>&& ConvexElements,
	TArray<physx::PxConvexMesh*>&& ConvexMeshes)
{
	ensure(CollisionTraceFlag != CTF_UseComplexAsSimple);
	ensure(ConvexElements.Num() == ConvexMeshes.Num());

	if (ConvexElements.Num() == 0)
	{
		if (Elements.Remove(Id) == 1)
		{
			bRebuildQueued = true;
		}
		return;
	}

	Elements.Add(Id, MakeUnique<FConvexElements>(InBounds, MoveTemp(ConvexElements), MoveTemp(ConvexMeshes)));
	bRebuildQueued = true;
}

void UVoxelWorldRootComponent::RebuildConvexCollision()
{	
	VOXEL_FUNCTION_COUNTER();
	ensure(CollisionTraceFlag != CTF_UseComplexAsSimple);

	{
		VOXEL_SCOPE_COUNTER("Update bounds");
		FBox LocalBox(ForceInit);
		for (auto& It : Elements)
		{
			LocalBox += It.Value->Bounds;
		}
		LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) : FBoxSphereBounds(ForceInit); // fallback to reset box sphere bounds
		UpdateBounds();
	}
	
	// Create body setup
	GetBodySetup();

	// Note: we do not need to call ClearPhysicsMeshes, as we are setting the convex meshes manually & handling the ref count ourselves
	
	auto& ConvexElements = BodySetup->AggGeom.ConvexElems;

	int32 NumConvexElements = 0;
	{
		VOXEL_SCOPE_COUNTER("Count");
		for(auto& It : Elements)
		{
			NumConvexElements += It.Value->ConvexElements.Num();
		}
	}
	if (NumConvexElements == 0) return;

	{
		VOXEL_SCOPE_COUNTER("Lock");
		BodySetupLock.Lock();
	}

	ConvexElements.Reset();
	{
		VOXEL_SCOPE_COUNTER("Reserve");
		ConvexElements.Reserve(NumConvexElements);
	}
	
	{
		VOXEL_SCOPE_COUNTER("Create");
		for(auto& It : Elements)
		{
			auto& Part = *It.Value;
			for (int32 Index = 0; Index < Part.ConvexMeshes.Num(); Index++)
			{
				auto& NewElement = *new (ConvexElements) FKConvexElem();
				// No need to copy the vertices
				NewElement.ElemBox = Part.ConvexElements[Index].ElemBox;
				NewElement.SetConvexMesh(Part.ConvexMeshes[Index]);
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

#if ENGINE_MINOR_VERSION >= 24
class UMRMeshComponent
{
public:
	static void FinishCreatingPhysicsMeshes(UBodySetup* Body, const TArray<physx::PxConvexMesh*>& ConvexMeshes, const TArray<physx::PxConvexMesh*>& ConvexMeshesNegX, const TArray<physx::PxTriangleMesh*>& TriMeshes)
	{
		Body->FinishCreatingPhysicsMeshes_PhysX(ConvexMeshes, ConvexMeshesNegX, TriMeshes);
	}
};
#endif

void UVoxelWorldRootComponent::SetCookedTriMeshes(const TArray<physx::PxTriangleMesh*>& TriMeshes)
{
	VOXEL_FUNCTION_COUNTER();

	// Create body setup
	GetBodySetup();

#if ENGINE_MINOR_VERSION < 24
	BodySetup->FinishCreatingPhysicsMeshes({}, {}, TriMeshes);
#else
	UMRMeshComponent::FinishCreatingPhysicsMeshes(BodySetup, {}, {}, TriMeshes);
#endif
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

		FScopeLock Lock(&Component->BodySetupLock);
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (!(VisibilityMap & (1 << ViewIndex))) continue;
			BodySetup->AggGeom.GetAggGeom(
				GeomTransform,
				SimpleCollisionColor,
				SolidMaterialInstance,
				true,
				true,
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