// Copyright 2018 Phyronnaz

#include "VoxelPoolManager.h"
#include "VoxelThreadPool.h"
#include "Misc/QueuedThreadPool.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "VoxelPoolManager"

FVoxelPool::FVoxelPool(int MeshThreadCount)
	: MeshPool(FVoxelQueuedThreadPool::Allocate())
	, OctreeBuilderPool(FQueuedThreadPool::Allocate())
	, AsyncTasksPool(FQueuedThreadPool::Allocate())
{
	MeshPool->Create(MeshThreadCount, 1024 * 1024);
	OctreeBuilderPool->Create(1, 1024 * 1024);
	AsyncTasksPool->Create(1, 1024 * 1024);
}

FVoxelPool::~FVoxelPool()
{
	delete MeshPool;
	delete OctreeBuilderPool;
}

FVoxelPoolRef FVoxelPool::Create(int MeshThreadCount)
{
	return FVoxelPoolRef(new FVoxelPool(MeshThreadCount), false);
}

void FVoxelPool::CheckRefs()
{
	check(RefCount >= 0);
	if (RefCount == 0 && bWantDestroy)
	{
		delete Global;
		bWantDestroy = false;
		Global = nullptr;
	}
}

FVoxelPoolRef FVoxelPool::GetGlobalPool()
{
	RefCount++;
	return FVoxelPoolRef(Global, true);
}

void FVoxelPool::CreateGlobalVoxelPool(int MeshThreadCount)
{
	check(!Global);
	check(RefCount == 0);
	check(!bWantDestroy);
	Global = new FVoxelPool(MeshThreadCount);
}

void FVoxelPool::DestroyGlobalVoxelPool()
{
	bWantDestroy = true;
	CheckRefs();
}

bool FVoxelPool::IsGlobalVoxelPoolCreated()
{
	return Global != nullptr;
}

FVoxelPool* FVoxelPool::Global = nullptr;
int FVoxelPool::RefCount = 0;
bool FVoxelPool::bWantDestroy = false;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelPoolRef::FVoxelPoolRef(FVoxelPool* Pool, bool bIsGlobal) 
	: Pool(Pool)
	, bIsGlobal(bIsGlobal)
{
	if (bIsGlobal)
	{
		FVoxelPool::RefCount++;
	}
}

FVoxelPoolRef::~FVoxelPoolRef()
{
	if (bIsGlobal)
	{
		FVoxelPool::RefCount--;
		FVoxelPool::CheckRefs();
	}
}

void FVoxelPoolRef::Reset()
{
	if (bIsGlobal)
	{
		FVoxelPool::RefCount--;
		FVoxelPool::CheckRefs();
	}
	else
	{
		delete Pool;
	}
	Pool = nullptr;
	bIsGlobal = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelPoolManager::CreateGlobalVoxelPool(int MeshThreadCount)
{
	if (IsGlobalVoxelPoolCreated())
	{
		FMessageLog("PIE").Error(LOCTEXT("CreateGlobalVoxelPool", "CreateGlobalVoxelPool called but global pool already created!"));
		return;
	}
	FVoxelPool::CreateGlobalVoxelPool(FMath::Max(1, MeshThreadCount));
}

void UVoxelPoolManager::DestroyGlobalVoxelPool()
{
	if (!IsGlobalVoxelPoolCreated())
	{
		FMessageLog("PIE").Error(LOCTEXT("DestroyGlobalVoxelPool", "DestroyGlobalVoxelPool called but global pool isn't created!"));
		return;
	}
	FVoxelPool::DestroyGlobalVoxelPool();
}

bool UVoxelPoolManager::IsGlobalVoxelPoolCreated()
{
	return FVoxelPool::IsGlobalVoxelPoolCreated();
}

#undef LOCTEXT_NAMESPACE