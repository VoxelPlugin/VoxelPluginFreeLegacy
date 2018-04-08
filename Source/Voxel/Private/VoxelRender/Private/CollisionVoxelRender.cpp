// Copyright 2018 Phyronnaz

#include "CollisionVoxelRender.h"
#include "VoxelPrivate.h"
#include "QueuedThreadPool.h"
#include "CollisionMeshHandler.h"
#include "VoxelInvokerComponent.h"
#include "VoxelWorld.h"

DECLARE_CYCLE_STAT(TEXT("FCollisionVoxelRender::Tick"), STAT_FCollisionVoxelRender_Tick, STATGROUP_Voxel);

FCollisionVoxelRender::FCollisionVoxelRender(AVoxelWorld* World, AActor* ChunksOwner)
	: IVoxelRender(World, ChunksOwner)
	, CollisionMeshHandlerThreadPool(FQueuedThreadPool::Allocate())
	, TimeSinceUpdate(0)
	, bNeedToEndCollisionsTasks(false)
{
	CollisionMeshHandlerThreadPool->Create(World->GetCollisionsThreadCount(), 1024 * 1024);
}

FCollisionVoxelRender::~FCollisionVoxelRender()
{
	CollisionComponents.Reset(0);
	CollisionMeshHandlerThreadPool->Destroy();
	delete CollisionMeshHandlerThreadPool;
}

void FCollisionVoxelRender::Tick(float DeltaTime)
{
	TimeSinceUpdate += DeltaTime;
	if (TimeSinceUpdate > 1 / World->GetCollisionsUpdateRate())
	{
		SCOPE_CYCLE_COUNTER(STAT_FCollisionVoxelRender_Tick);

		TimeSinceUpdate = 0;
		for (auto& Handler : CollisionComponents)
		{
			if (Handler->IsValid())
			{
				if (!bNeedToEndCollisionsTasks)
				{
					Handler->StartTasksTick();
				}
				else
				{
					Handler->EndTasksTick();
				}
			}
			else
			{
				Handler->Destroy();
				Handler.Reset();
			}
		}
		bNeedToEndCollisionsTasks = !bNeedToEndCollisionsTasks;

		Invokers.RemoveAll([](TWeakObjectPtr<UVoxelInvokerComponent> Invoker) { return !Invoker.IsValid(); });
		CollisionComponents.RemoveAll([](TSharedPtr<FCollisionMeshHandler> P) { return !P.IsValid(); });
	}
}

void FCollisionVoxelRender::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	if (Invoker.IsValid())
	{
		if (Invoker->UseForRender())
		{
			Invokers.Add(Invoker);
		}
		if (Invoker->UseForCollisions())
		{
			CollisionComponents.Add(MakeShared<FCollisionMeshHandler>(Invoker, World, ChunksOwner, CollisionMeshHandlerThreadPool));
		}
	}
}

void FCollisionVoxelRender::UpdateBoxInternal(const FIntBox& Box)
{
	for (auto Handler : CollisionComponents)
	{
		Handler->UpdateInBox(Box);
	}
}
