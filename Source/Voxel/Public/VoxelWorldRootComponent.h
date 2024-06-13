// Copyright Voxel Plugin SAS. All Rights Reserved.

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
	TObjectPtr<UBodySetup> BodySetup;

	FBoxSphereBounds LocalBounds;

	// For debug draw
	FCriticalSection BodySetupLock;

	friend class FVoxelRenderSimpleCollisionSceneProxy;
};