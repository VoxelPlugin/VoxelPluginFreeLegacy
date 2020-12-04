// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsyncPhysicsCooker.h"

#if WITH_CHAOS
namespace Chaos
{
	class FTriangleMeshImplicitObject;
}

class IPhysXCooking;

class FVoxelAsyncPhysicsCooker_Chaos : public IVoxelAsyncPhysicsCooker
{
	GENERATED_VOXEL_ASYNC_WORK_BODY(FVoxelAsyncPhysicsCooker_Chaos)

public:
	explicit FVoxelAsyncPhysicsCooker_Chaos(UVoxelProceduralMeshComponent* Component);

protected:
	//~ Begin IVoxelAsyncPhysicsCooker Interface
	virtual bool Finalize(UBodySetup& BodySetup, FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage) override;
	virtual void CookMesh() override;
	//~ End IVoxelAsyncPhysicsCooker Interface
	
private:
	void CreateTriMesh();

	TArray<TSharedPtr<Chaos::FTriangleMeshImplicitObject, ESPMode::ThreadSafe>> TriMeshes;
};
#endif