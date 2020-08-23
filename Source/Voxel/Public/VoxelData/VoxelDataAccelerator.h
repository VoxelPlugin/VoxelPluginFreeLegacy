// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

class FVoxelData;
class FVoxelDataOctreeLeaf;
class FVoxelDataOctreeBase;

namespace FVoxelDataAcceleratorParameters
{
	VOXEL_API int32 GetDefaultCacheSize();
	VOXEL_API bool GetUseAcceleratorMap();
	VOXEL_API bool GetShowStats();
}
	
template<typename TData>
class TVoxelDataAccelerator
{
public:
	TData& Data;
	const FVoxelIntBox Bounds;
	const int32 CacheSize;
	const bool bUseAcceleratorMap;

	static constexpr bool bIsConst = TIsConst<TData>::Value;

	// Will not build an accelerator map. Access can be done anywhere
	explicit TVoxelDataAccelerator(TData& Data, int32 CacheSize = FVoxelDataAcceleratorParameters::GetDefaultCacheSize());
	// Will build an accelerator map. Access can only be done in Bounds
	// Map source can't be used when TData is not const
	TVoxelDataAccelerator(TData& Data, const FVoxelIntBox& Bounds, const TVoxelDataAccelerator<TData>* MapSource = nullptr, int32 CacheSize = FVoxelDataAcceleratorParameters::GetDefaultCacheSize());
	~TVoxelDataAccelerator();

	// Copying can mess up the cache
	UE_NONCOPYABLE(TVoxelDataAccelerator);

public:
	template<typename T>
	T GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;

public:
	v_flt GetFloatValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, bool* bIsGeneratorValue = nullptr) const;

	FORCEINLINE v_flt GetFloatValue(const FVoxelVector& P, int32 LOD, bool* bIsGeneratorValue = nullptr) const
	{
		return GetFloatValue(P.X, P.Y, P.Z, LOD, bIsGeneratorValue);
	}

public:
	template<typename T>
	T Get(int32 X, int32 Y, int32 Z, int32 LOD) const;

	template<typename T>
	FORCEINLINE T Get(const FIntVector& P, int32 LOD) const
	{
		return Get<T>(P.X, P.Y, P.Z, LOD);
	}

	FORCEINLINE FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelValue>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelValue GetValue(const FIntVector& P, int32 LOD) const { return Get<FVoxelValue>(P, LOD); }

	FORCEINLINE FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z, int32 LOD) const { return Get<FVoxelMaterial>(X, Y, Z, LOD); }
	FORCEINLINE FVoxelMaterial GetMaterial(const FIntVector& P, int32 LOD) const { return Get<FVoxelMaterial>(P, LOD); }

public:
	// Returns if value was set, or if it was out of the world
	
	template<typename T>
	FORCEINLINE bool Set(int32 X, int32 Y, int32 Z, const T& Value)
	{
		return SetImpl<T>(X, Y, Z, [&](auto& InValue) { InValue = Value; });
	}
	template<typename T>
	FORCEINLINE bool Set(const FIntVector& P, const T& Value)
	{
		return Set<T>(P.X, P.Y, P.Z, Value);
	}

	template<typename TDummy = void>
	FORCEINLINE bool SetValue(int32 X, int32 Y, int32 Z, FVoxelValue Value) { return Set<FVoxelValue>(X, Y, Z, Value); }
	template<typename TDummy = void>
	FORCEINLINE bool SetValue(const FIntVector& P, FVoxelValue Value) { return Set<FVoxelValue>(P, Value); }

	template<typename TDummy = void>
	FORCEINLINE bool SetMaterial(int32 X, int32 Y, int32 Z, FVoxelMaterial Material) { return Set<FVoxelMaterial>(X, Y, Z, Material); }
	template<typename TDummy = void>
	FORCEINLINE bool SetMaterial(const FIntVector& P, FVoxelMaterial Material) { return Set<FVoxelMaterial>(P, Material); }

public:
	template<typename T, typename TLambda>
	FORCEINLINE bool Edit(int32 X, int32 Y, int32 Z, TLambda Lambda)
	{
		return SetImpl<T>(X, Y, Z, Lambda);
	}
	template<typename T, typename TLambda>
	FORCEINLINE bool Edit(const FIntVector& P, TLambda Lambda)
	{
		return Edit<T>(P.X, P.Y, P.Z, Lambda);
	}

	template<typename TLambda>
	FORCEINLINE bool EditValue(int32 X, int32 Y, int32 Z, TLambda Lambda) { return Edit<FVoxelValue>(X, Y, Z, Lambda); }
	template<typename TLambda>
	FORCEINLINE bool EditValue(const FIntVector& P, TLambda Lambda) { return Edit<FVoxelValue>(P, Lambda); }

	template<typename TLambda>
	FORCEINLINE bool EditMaterial(int32 X, int32 Y, int32 Z, TLambda Lambda) { return Edit<FVoxelMaterial>(X, Y, Z, Lambda); }
	template<typename TLambda>
	FORCEINLINE bool EditMaterial(const FIntVector& P, TLambda Lambda) { return Edit<FVoxelMaterial>(P, Lambda); }

private:
	struct FCacheEntry
	{
		FVoxelDataOctreeBase* Octree;
		uint64 LastAccessTime;
	};
	mutable TNoGrowArray<FCacheEntry> CacheEntries;
	mutable uint64 GlobalTime = 0;

#if VOXEL_DATA_ACCELERATOR_STATS
	mutable uint32 NumGet = 0;
	mutable uint32 NumSet = 0;

	mutable uint32 NumCacheTopAccess = 0;
	mutable uint32 NumCacheTopMiss = 0;

	mutable uint32 NumCacheAllAccess = 0;
	mutable uint32 NumCacheAllMiss = 0;

	mutable uint32 NumMapAccess = 0;
	mutable uint32 NumMapMiss = 0;
	
	mutable uint32 NumOutOfWorld = 0;
#endif

	using FAcceleratorMap = TMap<FIntVector, FVoxelDataOctreeLeaf*>;
	using FConstAcceleratorMap = typename TChooseClass<bIsConst, const FAcceleratorMap, FAcceleratorMap>::Result;
	
	// Map from Leaf.GetMin() to &Leaf
	const TVoxelSharedPtr<FConstAcceleratorMap> AcceleratorMap;

	template<typename T>
	auto GetImpl(int32 X, int32 Y, int32 Z, T UseOctree) const;

	template<typename T, typename TLambda>
	bool SetImpl(int32 X, int32 Y, int32 Z, TLambda EditValue) const;

	FVoxelDataOctreeBase* GetOctreeFromCache_CheckTopOnly(int32 X, int32 Y, int32 Z) const;
	FVoxelDataOctreeBase* GetOctreeFromCache_CheckAll(int32 X, int32 Y, int32 Z) const;
	FVoxelDataOctreeBase* GetOctreeFromMap(int32 X, int32 Y, int32 Z) const;

	void StoreOctreeInCache(FVoxelDataOctreeBase& Octree) const;

	static TVoxelSharedRef<FAcceleratorMap> GetAcceleratorMap(const FVoxelData& Data, const FVoxelIntBox& Bounds);
};

class FVoxelMutableDataAccelerator : public TVoxelDataAccelerator<FVoxelData>
{
public:
	using TVoxelDataAccelerator<FVoxelData>::TVoxelDataAccelerator;
};

class FVoxelConstDataAccelerator : public TVoxelDataAccelerator<const FVoxelData>
{
public:
	using TVoxelDataAccelerator<const FVoxelData>::TVoxelDataAccelerator;
};