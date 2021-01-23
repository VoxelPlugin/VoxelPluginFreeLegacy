// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "PhysXIncludes.h"
#include "VoxelPhysXRef.h"
#include "PhysicsEngine/BodySetup.h"
#include "Components/PrimitiveComponent.h"
#include "VoxelWorldRootComponent.generated.h"

class UVoxelWorldRootComponent;

struct FVoxelSimpleCollisionData
{
	FBox Bounds;
	TArray<FKBoxElem> BoxElems;
	TArray<FKConvexElem> ConvexElems;
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	TArray<TVoxelPhysXRef<PxConvexMesh>> ConvexMeshes;
#endif

	bool IsEmpty() const
	{
		return BoxElems.Num() == 0 && ConvexElems.Num() == 0;
	}
};

class FVoxelSimpleCollisionHandle
{
public:
	~FVoxelSimpleCollisionHandle();
	
	void SetCollisionData(TVoxelSharedPtr<FVoxelSimpleCollisionData> NewData);

private:
	const TWeakObjectPtr<UVoxelWorldRootComponent> Component;
	TVoxelSharedPtr<FVoxelSimpleCollisionData> Data;

	explicit FVoxelSimpleCollisionHandle(TWeakObjectPtr<UVoxelWorldRootComponent> Component);

	friend class UVoxelWorldRootComponent;
};

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
	TVoxelSharedRef<FVoxelSimpleCollisionHandle> CreateHandle();
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	void SetCookedTriMeshes(const TArray<physx::PxTriangleMesh*>& TriMeshes);
#endif

private:
	UPROPERTY(Transient)
	UBodySetup* BodySetup;

	FBoxSphereBounds LocalBounds;

	// For debug draw
	FCriticalSection BodySetupLock;
	
	bool bRebuildQueued = false;
	TArray<TVoxelWeakPtr<FVoxelSimpleCollisionHandle>> SimpleCollisionHandles;
	
	void RebuildConvexCollision();

	friend class FVoxelSimpleCollisionHandle;
	friend class FVoxelRenderSimpleCollisionSceneProxy;
};