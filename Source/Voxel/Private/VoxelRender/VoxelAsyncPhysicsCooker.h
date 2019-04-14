// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "VoxelThreadPool.h"

class IPhysXCooking;
class UBodySetup;
class UVoxelProceduralMeshComponent;

namespace physx
{
	class PxCooking;
	class PxTriangleMesh;
	class PxConvexMesh;
	class PxHeightField;
	class PxFoundation;
	struct PxCookingParams;
}

class FVoxelAsyncPhysicsCooker : public FVoxelAsyncWork
{
public:
	const uint64 UniqueId;

	TArray<physx::PxConvexMesh*> OutNonMirroredConvexMeshes;
	TArray<physx::PxConvexMesh*> OutMirroredConvexMeshes;
	TArray<physx::PxTriangleMesh*> OutTriangleMeshes;
	FBodySetupUVInfo OutUVInfo;

	FVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component, UBodySetup* BodySetup);

	inline bool HasSomethingToCook() const
	{
		return CookInfo.bCookTriMesh || CookInfo.bCookNonMirroredConvex || CookInfo.bCookMirroredConvex;
	}
	inline bool IsSuccessful() const
	{
		return SuccessCounter.GetValue() > 0;
	}
	
protected:
	virtual void DoWork() override;

private:
	bool Cook();
	void CreateConvexElements(const TArray<TArray<FVector>>& Elements, TArray<physx::PxConvexMesh*>& OutConvexMeshes, bool bFlipped);

	IPhysXCooking* const PhysXCooking;
	TWeakObjectPtr<UVoxelProceduralMeshComponent> const Component;
	FCookBodySetupInfo CookInfo;
	FThreadSafeCounter SuccessCounter;
};