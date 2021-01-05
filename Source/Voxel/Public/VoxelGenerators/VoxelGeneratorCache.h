// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelGCObject.h"
#include "VoxelSubsystem.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorCache.generated.h"

UCLASS()
class VOXEL_API UVoxelGeneratorCacheSubsystemProxy : public UVoxelStaticSubsystemProxy
{
	GENERATED_BODY()
	GENERATED_VOXEL_SUBSYSTEM_PROXY_BODY(FVoxelGeneratorCacheSubsystem);
};

class VOXEL_API FVoxelGeneratorCacheSubsystem : public IVoxelSubsystem
{
public:
	GENERATED_VOXEL_SUBSYSTEM_BODY(UVoxelGeneratorCacheSubsystemProxy);

	TVoxelSharedPtr<FVoxelGeneratorCache> GetCache() const { return GeneratorCache; }

	//~ Begin IVoxelSubsystem Interface
	virtual void Create() override;
	virtual void PreDestructor() override;
	//~ End IVoxelSubsystem Interface
	
private:
	mutable TVoxelSharedPtr<FVoxelGeneratorCache> GeneratorCache;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if CPP
class VOXEL_API FVoxelGeneratorCache : public FVoxelGCObject, public TVoxelSharedFromThis<FVoxelGeneratorCache>
{
public:
	VOXEL_SUBSYSTEM_FWD(FVoxelGeneratorCacheSubsystem, GetCache);
	
	static TVoxelSharedRef<FVoxelGeneratorCache> Create(const FVoxelGeneratorInit& Init, TVoxelWeakPtr<FVoxelRuntime> Runtime = nullptr);
	
public:
	TVoxelSharedRef<FVoxelGeneratorInstance> MakeGeneratorInstance(FVoxelGeneratorPicker Picker);
	TVoxelSharedRef<FVoxelTransformableGeneratorInstance> MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker);
	
	UVoxelGeneratorInstanceWrapper* K2_MakeGeneratorInstance(FVoxelGeneratorPicker Picker);
	UVoxelTransformableGeneratorInstanceWrapper* K2_MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker);
	
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGeneratorRecompiled, UVoxelGenerator*);
	static FOnGeneratorRecompiled OnGeneratorRecompiled;

private:
	void OnGeneratorRecompiledImpl(UVoxelGenerator* Generator);
	
protected:
	//~ Begin FVoxelGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return "FVoxelGeneratorCache"; }
	//~ End FVoxelGCObject Interface
	
private:
	FVoxelGeneratorCache() = default;
	
	FVoxelGeneratorInit GeneratorInit;
	
	TMap<FVoxelGeneratorPicker, TVoxelSharedPtr<FVoxelGeneratorInstance>> Cache;
	TMap<FVoxelTransformableGeneratorPicker, TVoxelSharedPtr<FVoxelTransformableGeneratorInstance>> TransformableCache;
	
	TMap<TVoxelWeakPtr<FVoxelGeneratorInstance>, UVoxelGeneratorInstanceWrapper*> WrapperCache;
	TMap<TVoxelWeakPtr<FVoxelTransformableGeneratorInstance>, UVoxelTransformableGeneratorInstanceWrapper*> WrapperTransformableCache;

	template<typename T>
	auto& GetCache();
	
	template<typename T>
	auto& GetWrapperCache();
	
	template<typename T>
	TVoxelSharedRef<typename T::FInstance> MakeGeneratorInstanceImpl(T Picker);
	
	template<typename T>
	typename T::UWrapper* K2_MakeGeneratorInstanceImpl(T Picker);
};

template<> inline auto& FVoxelGeneratorCache::GetCache<FVoxelGeneratorPicker>() { return Cache; }
template<> inline auto& FVoxelGeneratorCache::GetCache<FVoxelTransformableGeneratorPicker>() { return TransformableCache; }

template<> inline auto& FVoxelGeneratorCache::GetWrapperCache<FVoxelGeneratorPicker>() { return WrapperCache; }
template<> inline auto& FVoxelGeneratorCache::GetWrapperCache<FVoxelTransformableGeneratorPicker>() { return WrapperTransformableCache; }
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(BlueprintType)
class VOXEL_API UVoxelGeneratorCache : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * Creates (or reuse if possible) a new generator instance
	 *
	 * Among other things, this is required for DataItemActors to reuse generators, which allows for smaller update when moving them
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	UVoxelGeneratorInstanceWrapper* MakeGeneratorInstance(FVoxelGeneratorPicker Picker) const;
	
	/**
	 * Creates (or reuse if possible) a new generator instance
	 *
	 * Among other things, this is required for DataItemActors to reuse generators, which allows for smaller update when moving them
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	UVoxelTransformableGeneratorInstanceWrapper* MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const;
		
public:
	TVoxelSharedPtr<FVoxelGeneratorCache> GeneratorCache;
};