// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "PhysXIncludes.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelWorldRootComponent.generated.h"

DECLARE_UNIQUE_VOXEL_ID(FVoxelProcMeshComponentId);

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
template<typename T>
class TVoxelPhysXRef
{
public:
	TVoxelPhysXRef() = default;
	TVoxelPhysXRef(T* Ptr)
	{
		if (Ptr)
		{
			Impl = MakeVoxelShared<FImpl>(*Ptr);
		}
	}

	T* Get() const { return Impl ? &Impl->Ptr : nullptr; }

private:
	struct FImpl
	{
		T& Ptr;
		
		FImpl(T& Ptr)
			: Ptr(Ptr)
		{
			Ptr.acquireReference();
		}
		~FImpl()
		{
			Ptr.release();
		}
	};
	TVoxelSharedPtr<FImpl> Impl;
};

struct FVoxelSimpleCollisionData
{
	FBox Bounds;
	TArray<FKBoxElem> BoxElems;
	TArray<FKConvexElem> ConvexElems;
	TArray<TVoxelPhysXRef<PxConvexMesh>> ConvexMeshes;
};
#endif

UCLASS(editinlinenew, ShowCategories = (VirtualTexture))
class VOXEL_API UVoxelWorldRootComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UVoxelWorldRootComponent();
	~UVoxelWorldRootComponent();
	
	ECollisionTraceFlag CollisionTraceFlag = {};

	//~ Begin UPrimitiveComponent Interface
	virtual UBodySetup* GetBodySetup() override final;
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override final;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual TArray<URuntimeVirtualTexture*> const& GetRuntimeVirtualTextures() const override;
	virtual void OnDestroyPhysicsState() override;
	//~ End UPrimitiveComponent Interface

	// Only need to tick when created
	void TickWorldRoot();

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	void UpdateSimpleCollision(FVoxelProcMeshComponentId Id, FVoxelSimpleCollisionData&& SimpleCollision);
	void SetCookedTriMeshes(const TArray<physx::PxTriangleMesh*>& TriMeshes);
#endif

private:
	UPROPERTY(Transient)
	UBodySetup* BodySetup;

	FBoxSphereBounds LocalBounds;

	// For debug draw
	FCriticalSection BodySetupLock;
	
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	TMap<FVoxelProcMeshComponentId, TUniquePtr<FVoxelSimpleCollisionData>> ProcMeshesSimpleCollision;

	bool bRebuildQueued = false;
	
	void RebuildConvexCollision();
#endif

	friend class FVoxelRenderSimpleCollisionSceneProxy;
};