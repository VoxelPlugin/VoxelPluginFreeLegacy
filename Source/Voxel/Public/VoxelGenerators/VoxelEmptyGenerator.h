// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"
#include "VoxelGenerators/VoxelGeneratorInstance.inl"
#include "VoxelGenerators/VoxelTransformableGeneratorHelper.h"
#include "VoxelEmptyGenerator.generated.h"

class UVoxelEmptyGenerator;

class FVoxelEmptyGeneratorInstance : public TVoxelGeneratorInstanceHelper<FVoxelEmptyGeneratorInstance, UVoxelEmptyGenerator>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<FVoxelEmptyGeneratorInstance, UVoxelEmptyGenerator>;
	
	// WorldUpGenerator is used by VoxelPhysics for new parts
	explicit FVoxelEmptyGeneratorInstance(v_flt Value = 1, TVoxelSharedPtr<const FVoxelGeneratorInstance> WorldUpGenerator = nullptr)
		: Super(nullptr)
		, Value(Value)
		, WorldUpGenerator(WorldUpGenerator)
	{
	}

	//~ Begin FVoxelGeneratorInstance Interface
	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Value;
	}
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return FVoxelMaterial::Default();
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Value;
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return WorldUpGenerator.IsValid() ? WorldUpGenerator->GetUpVector(X, Y, Z) : FVector::UpVector;
	}
	//~ End FVoxelGeneratorInstance Interface

private:
	const v_flt Value;
	const TVoxelSharedPtr<const FVoxelGeneratorInstance> WorldUpGenerator = nullptr;
};

class FVoxelTransformableEmptyGeneratorInstance : public TVoxelTransformableGeneratorHelper<FVoxelEmptyGeneratorInstance>
{
public:
	explicit FVoxelTransformableEmptyGeneratorInstance(v_flt Value = 1)
		: TVoxelTransformableGeneratorHelper(MakeVoxelShared<FVoxelEmptyGeneratorInstance>(Value), false)
	{
	}
};

VOXEL_DEPRECATED(1.2, "Use FVoxelTransformableEmptyGeneratorInstance instead of FVoxelTransformableEmptyWorldGeneratorInstance")
typedef FVoxelTransformableEmptyGeneratorInstance FVoxelTransformableEmptyWorldGeneratorInstance;

/**
 * Empty world, can be used to remove voxels
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelEmptyGenerator : public UVoxelTransformableGenerator
{
	GENERATED_BODY()
	
public:
	//~ Begin UVoxelGenerator Interface
	TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override
	{
		return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
	}
	//~ End UVoxelGenerator Interface
};