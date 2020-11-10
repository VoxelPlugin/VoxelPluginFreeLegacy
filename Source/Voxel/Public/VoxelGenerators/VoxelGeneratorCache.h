// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Templates/SubclassOf.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGeneratorCache.generated.h"

class UVoxelGeneratorInstanceWrapper;
class UVoxelTransformableGeneratorInstanceWrapper;

#if CPP
class IVoxelGeneratorCache
{
public:
	virtual ~IVoxelGeneratorCache() = default;
	
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> MakeNativeGeneratorInstance(FVoxelGeneratorPicker Picker) const = 0;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> MakeNativeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const = 0;
};
#endif

class VOXEL_API FVoxelEmptyGeneratorCache : public IVoxelGeneratorCache
{
public:
	const FVoxelGeneratorInit& GeneratorInit;

	explicit FVoxelEmptyGeneratorCache(const FVoxelGeneratorInit& GeneratorInit)
		: GeneratorInit(GeneratorInit)
	{
	}
	
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> MakeNativeGeneratorInstance(FVoxelGeneratorPicker Picker) const override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> MakeNativeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const override;
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelGeneratorCache
	: public UObject
#if CPP
	, public IVoxelGeneratorCache
#endif
{
public:
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
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> MakeNativeGeneratorInstance(FVoxelGeneratorPicker Picker) const override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> MakeNativeTransformableGeneratorInstance(FVoxelTransformableGeneratorPicker Picker) const override;

public:
	void SetGeneratorInit(const FVoxelGeneratorInit& NewInit);
	void ClearCache();
	
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

private:
	UPROPERTY()
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
	
	mutable TMap<FVoxelGeneratorPicker, FCache> Cache;
	mutable TMap<FVoxelTransformableGeneratorPicker, FTransformableCache> TransformableCache;
};