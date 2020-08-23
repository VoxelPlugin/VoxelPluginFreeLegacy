// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorInstance.inl"
#include "VoxelWorldGenerators/VoxelTransformableWorldGeneratorHelper.h"
#include "VoxelEmptyWorldGenerator.generated.h"

class UVoxelEmptyWorldGenerator;

class FVoxelEmptyWorldGeneratorInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelEmptyWorldGeneratorInstance, UVoxelEmptyWorldGenerator>
{
public:
	using Super = TVoxelWorldGeneratorInstanceHelper<FVoxelEmptyWorldGeneratorInstance, UVoxelEmptyWorldGenerator>;
	
	// WorldUpGenerator is used by VoxelPhysics for new parts
	explicit FVoxelEmptyWorldGeneratorInstance(v_flt Value = 1, TVoxelSharedPtr<const FVoxelWorldGeneratorInstance> WorldUpGenerator = nullptr)
		: Super(nullptr)
		, Value(Value)
		, WorldUpGenerator(WorldUpGenerator)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
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
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const v_flt Value;
	const TVoxelSharedPtr<const FVoxelWorldGeneratorInstance> WorldUpGenerator = nullptr;
};

class FVoxelTransformableEmptyWorldGeneratorInstance : public TVoxelTransformableWorldGeneratorHelper<FVoxelEmptyWorldGeneratorInstance>
{
public:
	explicit FVoxelTransformableEmptyWorldGeneratorInstance(v_flt Value = 1)
		: TVoxelTransformableWorldGeneratorHelper(MakeVoxelShared<FVoxelEmptyWorldGeneratorInstance>(Value), false)
	{
	}
};

/**
 * Empty world, can be used to remove voxels
 */
UCLASS(Blueprintable)
class VOXEL_API UVoxelEmptyWorldGenerator : public UVoxelTransformableWorldGenerator
{
	GENERATED_BODY()
	
public:
	//~ Begin UVoxelWorldGenerator Interface
	TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override
	{
		return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
	}
	//~ End UVoxelWorldGenerator Interface
};