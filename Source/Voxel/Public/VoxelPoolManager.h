// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelPoolManager.generated.h"

class FVoxelQueuedThreadPool;
class FQueuedThreadPool;
class FVoxelPool;

class VOXEL_API FVoxelPoolRef
{
public:
	FVoxelPoolRef() = default;
	~FVoxelPoolRef();

	inline FVoxelPool* Get() const { return Pool; }
	inline FVoxelPool* operator->() const { return Pool; }
	inline bool IsValid() const { return Pool != nullptr; }
	void Reset();

private:
	FVoxelPoolRef(FVoxelPool* Pool, bool bIsGlobal);
	
	FVoxelPool* Pool = nullptr;
	bool bIsGlobal = false;
	
	friend class FVoxelPool;
};

///////////////////////////////////////////////////////////////////////////////

class VOXEL_API FVoxelPool
{
public:
	FVoxelQueuedThreadPool* const MeshPool;
	FQueuedThreadPool* const OctreeBuilderPool;
	FQueuedThreadPool* const AsyncTasksPool;

private:
	FVoxelPool(int MeshThreadCount);
	~FVoxelPool();

public:
	static FVoxelPoolRef Create(int MeshThreadCount);
	static FVoxelPoolRef GetGlobalPool();
	
	static void CreateGlobalVoxelPool(int MeshThreadCount);
	static void DestroyGlobalVoxelPool();
	static bool IsGlobalVoxelPoolCreated();

private:
	static void CheckRefs();

	static FVoxelPool* Global;
	static int RefCount;
	static bool bWantDestroy;

	friend class FVoxelPoolRef;
};

///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelPoolManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Min 1
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void CreateGlobalVoxelPool(int MeshThreadCount = 2);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void DestroyGlobalVoxelPool();

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static bool IsGlobalVoxelPoolCreated();
};