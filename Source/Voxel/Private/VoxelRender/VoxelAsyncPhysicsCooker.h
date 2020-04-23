// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelAsyncWork.h"
#include "VoxelPriorityHandler.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/WeakObjectPtrTemplates.h"

struct FVoxelProcMeshBuffers;
class IVoxelProceduralMeshComponent_PhysicsCallbackHandler;
class IPhysXCooking;
class UBodySetup;
class UVoxelProceduralMeshComponent;

class FVoxelAsyncPhysicsCooker : public FVoxelAsyncWork
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

	explicit FVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component);

	inline bool IsSuccessful() const
	{
		return ErrorCounter.GetValue() == 0;
	}

private:
	~FVoxelAsyncPhysicsCooker() = default;

	template<typename T>
	friend struct TVoxelAsyncWorkDelete;
	
protected:
	//~ Begin FVoxelAsyncWork Interface
	virtual void DoWork() override;
	virtual void PostDoWork() override;
	virtual uint32 GetPriority() const override;
	//~ End FVoxelAsyncWork Interface
	
private:
	void CreateTriMesh();
	void CreateConvexMesh();
	void DecomposeMeshToHulls();
	EPhysXMeshCookFlags GetCookFlags() const;

	IPhysXCooking* const PhysXCooking;
	FThreadSafeCounter ErrorCounter;

public:
	struct FCookResult
	{
		FBox ConvexBounds;
		TArray<FKConvexElem> ConvexElems;
		TArray<physx::PxConvexMesh*> ConvexMeshes;
		TArray<physx::PxTriangleMesh*> TriangleMeshes;

		uint64 TriangleMeshesMemoryUsage = 0;
	};
	FCookResult CookResult;
};