// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelAsyncWork.h"
#include "VoxelPriorityHandler.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/WeakObjectPtrTemplates.h"

struct FVoxelProcMeshBuffers;
struct FVoxelProceduralMeshComponentMemoryUsage;
class UBodySetup;
class UVoxelProceduralMeshComponent;
class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;

class IVoxelAsyncPhysicsCooker : public FVoxelAsyncWork
{
public:
	const uint64 UniqueId;
	const TWeakObjectPtr<UVoxelProceduralMeshComponent> Component;
	const TVoxelWeakPtr<IVoxelProceduralMeshComponent_PhysicsCallbackHandler> PhysicsCallbackHandler;
	
	const int32 LOD;
	const ECollisionTraceFlag CollisionTraceFlag;
	const FVoxelPriorityHandler PriorityHandler;
	const bool bCleanCollisionMesh;
	const int32 NumConvexHullsPerAxis;
	const TArray<TVoxelSharedPtr<const FVoxelProcMeshBuffers>> Buffers;
	const FTransform LocalToRoot;

	explicit IVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component);

	static IVoxelAsyncPhysicsCooker* CreateCooker(UVoxelProceduralMeshComponent* Component);
	
public:
	//~ Begin IVoxelAsyncPhysicsCooker Interface
	virtual bool Finalize(UBodySetup& BodySetup, FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage) = 0;
protected:
	virtual void CookMesh() = 0;
	//~ End IVoxelAsyncPhysicsCooker Interface

protected:
	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() override;
	virtual void PostDoWork() override;
	virtual uint32 GetPriority() const override;
	//~ End FVoxelAsyncWork Interface
};