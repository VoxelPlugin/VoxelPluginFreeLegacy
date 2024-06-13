// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelWorldRootComponent.h"
#include "VoxelMinimal.h"
#if VOXEL_ENGINE_VERSION < 504
#include "PhysXIncludes.h"
#endif
#include "PrimitiveSceneProxy.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"

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

void UVoxelWorldRootComponent::TickWorldRoot()
{
	VOXEL_FUNCTION_COUNTER();
}

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