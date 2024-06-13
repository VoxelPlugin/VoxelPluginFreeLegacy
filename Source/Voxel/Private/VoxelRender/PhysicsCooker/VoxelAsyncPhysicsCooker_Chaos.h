// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelAsyncPhysicsCooker.h"

namespace Chaos
{
	class FTriangleMeshImplicitObject;
}

class IPhysXCooking;

class FVoxelAsyncPhysicsCooker_Chaos : public IVoxelAsyncPhysicsCooker
{
public:
	explicit FVoxelAsyncPhysicsCooker_Chaos(UVoxelProceduralMeshComponent* Component);

private:
	~FVoxelAsyncPhysicsCooker_Chaos() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;

protected:
	//~ Begin IVoxelAsyncPhysicsCooker Interface
	virtual bool Finalize(UBodySetup& BodySetup, FVoxelProceduralMeshComponentMemoryUsage& OutMemoryUsage) override;
	virtual void CookMesh() override;
	//~ End IVoxelAsyncPhysicsCooker Interface
	
private:
	void CreateTriMesh();

#if VOXEL_ENGINE_VERSION >= 504
	TArray<Chaos::FTriangleMeshImplicitObjectPtr> TriMeshes;
#else
	TArray<TSharedPtr<Chaos::FTriangleMeshImplicitObject, ESPMode::ThreadSafe>> TriMeshes;
#endif
};