// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "PhysicsEngine/BodySetup.h" // Can't forward decl anything with uobjects generated constructors...
#include "Components/PrimitiveComponent.h"
#include "VoxelWorldRootComponent.generated.h"

DECLARE_UNIQUE_VOXEL_ID(FVoxelProcMeshComponentId);

struct FVoxelSimpleCollisionData
{
	FBox Bounds;
	TArray<FKBoxElem> BoxElems;
	TArray<FKConvexElem> ConvexElems;
	TArray<physx::PxConvexMesh*> ConvexMeshes;
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
	struct FSimpleCollisionDataRef
	{
		const FVoxelSimpleCollisionData Data;

		FSimpleCollisionDataRef(FVoxelSimpleCollisionData&& Data);
		~FSimpleCollisionDataRef();
	};
	TMap<FVoxelProcMeshComponentId, TUniquePtr<FSimpleCollisionDataRef>> ProcMeshesSimpleCollision;

	bool bRebuildQueued = false;
	
	void RebuildConvexCollision();
#endif

	friend class FVoxelRenderSimpleCollisionSceneProxy;
};