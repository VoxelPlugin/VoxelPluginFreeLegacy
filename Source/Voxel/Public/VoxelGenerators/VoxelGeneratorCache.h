// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSubsystem.h"
#include "UObject/GCObject.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorCache.generated.h"

class UVoxelGeneratorInstanceWrapper;
class UVoxelTransformableGeneratorInstanceWrapper;

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
	//~ End IVoxelSubsystem Interface
	
private:
	mutable TVoxelSharedPtr<FVoxelGeneratorCache> GeneratorCache;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if CPP
class VOXEL_API FVoxelGeneratorCache : public FGCObject, public TVoxelSharedFromThis<FVoxelGeneratorCache>
{
public:
	VOXEL_SUBSYSTEM_FWD(FVoxelGeneratorCacheSubsystem, GetCache);
	
	static TVoxelSharedRef<FVoxelGeneratorCache> Create(const FVoxelGeneratorInit& Init);
	
public:
	UVoxelGeneratorInstanceWrapper* K2_MakeGeneratorInstance(FVoxelGeneratorPicker Picker);
	UVoxelTransformableGeneratorInstanceWrapper* K2_MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker);
	
	TVoxelSharedRef<FVoxelGeneratorInstance> MakeGeneratorInstance(FVoxelGeneratorPicker Picker);
	TVoxelSharedRef<FVoxelTransformableGeneratorInstance> MakeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker);

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGeneratorRecompiled, UVoxelGenerator*);
	static FOnGeneratorRecompiled OnGeneratorRecompiled;

private:
	void OnGeneratorRecompiledImpl(UVoxelGenerator* Generator);
	
protected:
	//~ Begin FGCObject Interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FGCObject Interface
	
private:
	FVoxelGeneratorCache() = default;
	
	FVoxelGeneratorInit GeneratorInit;

	struct FCache
	{
		UVoxelGeneratorInstanceWrapper* Wrapper = nullptr;
		TVoxelSharedPtr<FVoxelGeneratorInstance> Instance;
	};
	struct FTransformableCache
	{
		UVoxelTransformableGeneratorInstanceWrapper* Wrapper = nullptr;
		TVoxelSharedPtr<FVoxelTransformableGeneratorInstance> Instance;
	};
	
	TMap<FVoxelGeneratorPicker, FCache> Cache;
	TMap<FVoxelTransformableGeneratorPicker, FTransformableCache> TransformableCache;
};
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