// Copyright 2020 Phyronnaz

#include "IVoxelPool.h"
#include "VoxelMinimal.h"
#include "Engine/World.h"

TMap<TWeakObjectPtr<UWorld>, TVoxelSharedPtr<IVoxelPool>> IVoxelPool::WorldsPools;
TVoxelSharedPtr<IVoxelPool> IVoxelPool::GlobalPool;

TVoxelSharedPtr<IVoxelPool> IVoxelPool::GetWorldPool(UWorld* World)
{
	return WorldsPools.FindRef(World);
}

TVoxelSharedPtr<IVoxelPool> IVoxelPool::GetGlobalPool()
{
	return GlobalPool;
}

TVoxelSharedPtr<IVoxelPool> IVoxelPool::GetPoolForWorld(UWorld* World)
{
	if (auto Pool = GetWorldPool(World))
	{
		return Pool;
	}
	return GetGlobalPool();
}

///////////////////////////////////////////////////////////////////////////////

void IVoxelPool::SetWorldPool(UWorld* World, const TVoxelSharedRef<IVoxelPool>& Pool, const FString& Creator)
{
	ensure(World);
	ensure(!WorldsPools.Contains(World));
	
	WorldsPools.Add(World, Pool);

	LOG_VOXEL(Log, TEXT("Voxel Pool created by %s for world %s"), *Creator, *World->GetName());
}

void IVoxelPool::SetGlobalPool(const TVoxelSharedRef<IVoxelPool>& Pool, const FString& Creator)
{
	ensure(!GlobalPool.IsValid());

	GlobalPool = Pool;

	LOG_VOXEL(Log, TEXT("Global Voxel Pool created by %s"), *Creator);
}

///////////////////////////////////////////////////////////////////////////////

void IVoxelPool::DestroyWorldPool(UWorld* World)
{
	if (ensure(WorldsPools.Contains(World)))
	{
		WorldsPools.Remove(World);
		LOG_VOXEL(Log, TEXT("Voxel Pool destroyed for %s"), *World->GetName());
	}
}

void IVoxelPool::DestroyGlobalPool()
{
	if (ensure(GlobalPool.IsValid()))
	{
		GlobalPool.Reset();
		LOG_VOXEL(Log, TEXT("Global Voxel Pool destroyed"));
	}
}

void IVoxelPool::Shutdown()
{
	// Make sure to delete them cleanly here, as else issues can arise with ReturnSynchEventToPool
	GlobalPool.Reset();
	WorldsPools.Reset();
}