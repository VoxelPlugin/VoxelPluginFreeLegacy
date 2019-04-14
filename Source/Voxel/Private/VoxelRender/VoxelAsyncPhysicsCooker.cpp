// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelAsyncPhysicsCooker.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelGlobals.h"

#include "Physics/IPhysXCooking.h"
#include "Physics/IPhysXCookingModule.h"
#include "Async/Async.h"
#include "PhysicsPublic.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelAsyncPhysicsCooker::WaitForCompletion"), STAT_FVoxelAsyncPhysicsCooker_WaitForCompletion, STATGROUP_Voxel);

IPhysXCooking* GetPhysXCooking()
{
	static IPhysXCookingModule* PhysXCookingModule = nullptr;
	if (!PhysXCookingModule)
	{
		PhysXCookingModule = GetPhysXCookingModule();
	}
	return PhysXCookingModule->GetPhysXCooking();
}

static const FName PhysXFormat = FPlatformProperties::GetPhysicsFormat();

inline uint64 GetUniqueId()
{
	static uint64 Id;
	return Id++;
}

FVoxelAsyncPhysicsCooker::FVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component, UBodySetup* BodySetup)
	: FVoxelAsyncWork("AsyncPhysicsCooker", Component->Priority)
	, UniqueId(GetUniqueId())
	, PhysXCooking(GetPhysXCooking())
	, Component(Component)
{
	check(IsInGameThread());
	BodySetup->GetCookInfo(CookInfo, EPhysXMeshCookFlags::Default);
}

void FVoxelAsyncPhysicsCooker::DoWork()
{
	if (!IsCanceled())
	{
		if (Cook())
		{
			SuccessCounter.Increment();
		}
	}
	if (!IsCanceled())
	{
		AsyncTask(ENamedThreads::GameThread, [Component = Component, UniqueId = UniqueId]()
		{
			if (Component.IsValid())
			{
				Component->AsyncPhysicsCookerCallback(UniqueId);
			}
		});
	}
}

#define CANCEL_IF_NEEDED() if (IsCanceled()) { return false; }

bool FVoxelAsyncPhysicsCooker::Cook()
{
	CANCEL_IF_NEEDED();
	CreateConvexElements(CookInfo.NonMirroredConvexVertices, OutNonMirroredConvexMeshes, false);
	CANCEL_IF_NEEDED();
	CreateConvexElements(CookInfo.MirroredConvexVertices, OutMirroredConvexMeshes, true);
	CANCEL_IF_NEEDED();

	if (CookInfo.bCookTriMesh && !CookInfo.bTriMeshError)
	{
		OutTriangleMeshes.AddZeroed();
		bool bError = !PhysXCooking->CreateTriMesh(
			PhysXFormat, 
			CookInfo.TriMeshCookFlags, 
			CookInfo.TriangleMeshDesc.Vertices, 
			CookInfo.TriangleMeshDesc.Indices, 
			CookInfo.TriangleMeshDesc.MaterialIndices, 
			CookInfo.TriangleMeshDesc.bFlipNormals, 
			OutTriangleMeshes.Last());
		if (bError)
		{
			UE_LOG(LogVoxel, Warning, TEXT("Failed to cook TriMesh: %s."), *CookInfo.OuterDebugName);
			return false;
		}
		CANCEL_IF_NEEDED();
	}
    return true;
}

void FVoxelAsyncPhysicsCooker::CreateConvexElements(const TArray<TArray<FVector>>& Elements, TArray<PxConvexMesh*>& OutConvexMeshes, bool bFlipped)
{
	OutMirroredConvexMeshes.Reserve(Elements.Num());
	for (int32 ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex)
	{
		OutConvexMeshes.AddZeroed();
		const EPhysXCookingResult Result = PhysXCooking->CreateConvex(PhysXFormat, CookInfo.ConvexCookFlags, Elements[ElementIndex], OutConvexMeshes.Last());
		switch (Result)
		{
		case EPhysXCookingResult::Succeeded:
			break;
		case EPhysXCookingResult::Failed:
			UE_LOG(LogVoxel, Warning, TEXT("Failed to cook convex: %s %d (FlipX:%d). The remaining elements will not get cooked."), *CookInfo.OuterDebugName, ElementIndex, bFlipped ? 1 : 0);
			break;
		case EPhysXCookingResult::SucceededWithInflation:
			if (!CookInfo.bConvexDeformableMesh)
			{
				UE_LOG(LogVoxel, Warning, TEXT("Cook convex: %s %d (FlipX:%d) failed but succeeded with inflation.  The mesh should be looked at."), *CookInfo.OuterDebugName, ElementIndex, bFlipped ? 1 : 0);
			}
			else
			{
				UE_LOG(LogVoxel, Log, TEXT("Cook convex: %s %d (FlipX:%d) required inflation. You may wish to adjust the mesh so this is not necessary."), *CookInfo.OuterDebugName, ElementIndex, bFlipped ? 1 : 0);
			}
			break;
		default:
			check(false);
		}
	}
}