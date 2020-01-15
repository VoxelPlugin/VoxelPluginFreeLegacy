// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/BodySetup.h" // Can't forward decl anything with uobjects generated constructors...
#include "Components/PrimitiveComponent.h"
#include "VoxelWorldRootComponent.generated.h"

UCLASS(editinlinenew)
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
	//~ End UPrimitiveComponent Interface

	// Only need to tick when created
	void TickWorldRoot();

private:
	UPROPERTY(Transient)
	UBodySetup* BodySetup;

	FBoxSphereBounds LocalBounds;

	struct FConvexElements
	{
		const FBox Bounds;
		const TArray<FKConvexElem> ConvexElements;
		const TArray<physx::PxConvexMesh*> ConvexMeshes;

		FConvexElements(const FBox& InBounds, TArray<FKConvexElem>&& InConvexElements, TArray<physx::PxConvexMesh*>&& InConvexMeshes);
		~FConvexElements();
	};
	TMap<uint64, TUniquePtr<FConvexElements>> Elements;

	// For debug draw
	FCriticalSection BodySetupLock;

	bool bRebuildQueued = false;
	
	void RebuildConvexCollision();

	friend class FVoxelRenderSimpleCollisionSceneProxy;

public:
	void UpdateConvexCollision(uint64 Id, const FBox& Bounds, TArray<FKConvexElem>&& ConvexElements, TArray<physx::PxConvexMesh*>&& ConvexMeshes);
};