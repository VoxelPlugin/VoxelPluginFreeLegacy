// Copyright 2020 Phyronnaz

#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker.h"
#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker_PhysX.h"
#include "VoxelRender/PhysicsCooker/VoxelAsyncPhysicsCooker_Chaos.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/IVoxelProceduralMeshComponent_PhysicsCallbackHandler.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "PhysicsEngine/PhysicsSettings.h"

double GTotalVoxelCollisionCookingTime = 0;

static TAutoConsoleVariable<int32> CVarLogCollisionCookingTimes(
	TEXT("voxel.collision.LogCookingTimes"),
	0,
	TEXT("If true, will log the time it took to cook the voxel meshes collisions"),
	ECVF_Default);

static FAutoConsoleCommand CmdLogTotalCollisionCookingTime(
    TEXT("voxel.collision.LogTotalCookingTime"),
    TEXT("Log the accumulated total spent computing collision. Also see voxel.collision.ClearTotalCookingTime"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
	    LOG_VOXEL(Log, TEXT("Total collision cooking time: %fs"), GTotalVoxelCollisionCookingTime);
    }));

static FAutoConsoleCommand CmdClearTotalCollisionCookingTime(
    TEXT("voxel.collision.ClearTotalCookingTime"),
    TEXT("Clear the accumulated total spent computing collision. Also see voxel.collision.LogTotalCookingTime"),
    FConsoleCommandDelegate::CreateLambda([]()
    {
    	GTotalVoxelCollisionCookingTime = 0;
	    LOG_VOXEL(Log, TEXT("Total collision cooking time cleared"));
    }));

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IVoxelAsyncPhysicsCooker::IVoxelAsyncPhysicsCooker(UVoxelProceduralMeshComponent* Component)
	: FVoxelAsyncWork(STATIC_FNAME("AsyncPhysicsCooker"), Component->PriorityDuration)
	, UniqueId(UNIQUE_ID())
	, Component(Component)
	, PhysicsCallbackHandler(Component->PhysicsCallbackHandler)
	, LOD(Component->LOD)
	, CollisionTraceFlag(
		Component->CollisionTraceFlag == ECollisionTraceFlag::CTF_UseDefault
		? ECollisionTraceFlag(UPhysicsSettings::Get()->DefaultShapeComplexity)
		: Component->CollisionTraceFlag)
	, PriorityHandler(Component->PriorityHandler)
	, bCleanCollisionMesh(Component->bCleanCollisionMesh)
	, NumConvexHullsPerAxis(Component->NumConvexHullsPerAxis)
	, Buffers([&]()
		{
			TArray<TVoxelSharedPtr<const FVoxelProcMeshBuffers>> TmpBuffers;
			TmpBuffers.Reserve(Component->ProcMeshSections.Num());
			for (auto& Section : Component->ProcMeshSections)
			{
				if (Section.Settings.bEnableCollisions)
				{
					TmpBuffers.Add(Section.Buffers);
				}
			}
			return TmpBuffers;
		}())
	, LocalToRoot(Component->GetRelativeTransform())
{
	check(IsInGameThread());
	ensure(CollisionTraceFlag != ECollisionTraceFlag::CTF_UseDefault);
	ensure(Buffers.Num() > 0);
}

IVoxelAsyncPhysicsCooker* IVoxelAsyncPhysicsCooker::CreateCooker(UVoxelProceduralMeshComponent* Component)
{
	check(Component);
#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
	return new FVoxelAsyncPhysicsCooker_PhysX(Component);
#elif WITH_CHAOS
	return new FVoxelAsyncPhysicsCooker_Chaos(Component);
#else
	return nullptr;
#endif
}

void IVoxelAsyncPhysicsCooker::DoWork()
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const double CookStartTime = FPlatformTime::Seconds();
	
	CookMesh();

	const double CookEndTime = FPlatformTime::Seconds();
	
	if (CVarLogCollisionCookingTimes.GetValueOnAnyThread() != 0)
	{
		LOG_VOXEL(Log, TEXT("Collisions cooking took %fms"), (CookEndTime - CookStartTime) * 1000);
	}

	GTotalVoxelCollisionCookingTime += CookEndTime - CookStartTime;
}

void IVoxelAsyncPhysicsCooker::PostDoWork()
{
	auto Pinned = PhysicsCallbackHandler.Pin();
	if (Pinned.IsValid())
	{
		Pinned->CookerCallback(UniqueId, Component);
		FVoxelUtilities::DeleteOnGameThread_AnyThread(Pinned);
	}
}

uint32 IVoxelAsyncPhysicsCooker::GetPriority() const
{
	return PriorityHandler.GetPriority();
}