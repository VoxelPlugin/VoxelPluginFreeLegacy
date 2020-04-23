// Copyright 2020 Phyronnaz

#include "IVoxelPool.h"
#include "VoxelGlobals.h"
#include "Engine/World.h"

TMap<TWeakObjectPtr<UWorld>, IVoxelPool::FPool> IVoxelPool::GlobalMap;

TVoxelSharedPtr<IVoxelPool> IVoxelPool::GetGlobalPool(UWorld* World)
{
	GlobalMap.Remove(nullptr);
	return GlobalMap.FindChecked(World).Pool;
}

const FString& IVoxelPool::GetGlobalPoolCreator(UWorld* World)
{
	GlobalMap.Remove(nullptr);
	return GlobalMap.FindChecked(World).Creator;
}

void IVoxelPool::DestroyGlobalVoxelPool(UWorld* World)
{
	GlobalMap.Remove(nullptr);
	GlobalMap.Remove(World);
	LOG_VOXEL(Log, TEXT("Global pool destroyed for %s"), *World->GetName());
}

bool IVoxelPool::IsGlobalVoxelPoolCreated(UWorld* World)
{
	GlobalMap.Remove(nullptr);
	return GlobalMap.Contains(World);
}

void IVoxelPool::SetGlobalVoxelPool(UWorld* World, const TVoxelSharedPtr<IVoxelPool>& Pool, const FString& Creator)
{
	check(Pool.IsValid());

	GlobalMap.Remove(nullptr);
	checkf(!GlobalMap.Contains(World), TEXT("Global voxel pool already created!"));

	auto& NewPool = GlobalMap.Add(World);
	NewPool.Creator = Creator;
	NewPool.Pool = Pool;

	LOG_VOXEL(Log, TEXT("Global pool created by %s for %s"), *Creator, *World->GetName());
}