// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IPhysXCooking.h"
#include "VoxelWorldRootComponent.h"
#include "VoxelAsyncPhysicsCooker.h"

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
struct FKConvexElem;
class IPhysXCooking;

class FVoxelAsyncPhysicsCooker_PhysX : public IVoxelAsyncPhysicsCooker
{
	GENERATED_VOXEL_ASYNC_WORK_BODY(FVoxelAsyncPhysicsCooker_PhysX)

public:
	explicit FVoxelAsyncPhysicsCooker_PhysX(UVoxelProceduralMeshComponent* Component);

private:
	//~ Begin IVoxelAsyncPhysicsCooker Interface
	virtual bool Finalize(UBodySetup& BodySetup, FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage) override;
	virtual void CookMesh() override;
	//~ End IVoxelAsyncPhysicsCooker Interface
	
private:
	void CreateTriMesh();
	void CreateSimpleCollision();
	EPhysXMeshCookFlags GetCookFlags() const;

	IPhysXCooking* const PhysXCooking;
	FThreadSafeCounter ErrorCounter;

	struct FCookResult
	{
		FVoxelSimpleCollisionData SimpleCollisionData;

	    TArray<physx::PxTriangleMesh*> TriangleMeshes;
		uint64 TriangleMeshesMemoryUsage = 0;
	};
	FCookResult CookResult;
};
#endif