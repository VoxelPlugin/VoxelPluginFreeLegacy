// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGeneratorInstance.inl"
#include "VoxelEmptyWorldGenerator.generated.h"

class UVoxelEmptyWorldGenerator;

class FVoxelEmptyWorldGeneratorInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelEmptyWorldGeneratorInstance, UVoxelEmptyWorldGenerator>
{
public:
	FVoxelEmptyWorldGeneratorInstance() = default;
	explicit FVoxelEmptyWorldGeneratorInstance(TVoxelSharedPtr<const FVoxelWorldGeneratorInstance> WorldUpGenerator)
		: WorldUpGenerator(WorldUpGenerator)
	{
	}
	explicit FVoxelEmptyWorldGeneratorInstance(v_flt Value)
		: Value(Value)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Value;
	}
	inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return FVoxelMaterial::Default();
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Value;
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return WorldUpGenerator.IsValid() ? WorldUpGenerator->GetUpVector(X, Y, Z) : FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	const TVoxelSharedPtr<const FVoxelWorldGeneratorInstance> WorldUpGenerator = nullptr;
	const v_flt Value = 1;
};

class FVoxelTransformableEmptyWorldGeneratorInstance : public TVoxelTransformableWorldGeneratorInstanceHelper<FVoxelTransformableEmptyWorldGeneratorInstance, UVoxelEmptyWorldGenerator>
{
public:
	const FIntBox LocalBounds;
	
	explicit FVoxelTransformableEmptyWorldGeneratorInstance(FIntBox LocalBounds = FIntBox(-1, 2))
		: LocalBounds(LocalBounds)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	template<bool bCustomTransform>
	inline v_flt GetValueImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Items.IsEmpty() || LocalBounds.ContainsFloat(LocalToWorld.InverseTransformPosition(FVector(X, Y, Z))))
		{
			return 1;
		}
		else
		{
			const auto NextStack = Items.GetNextStack(X, Y, Z);
			return NextStack.Get<v_flt>(X, Y, Z, LOD);
		}
	}
	template<bool bCustomTransform>
	inline FVoxelMaterial GetMaterialImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Items.IsEmpty())
		{
			return FVoxelMaterial::Default();
		}
		else
		{
			const auto NextStack = Items.GetNextStack(X, Y, Z);
			return NextStack.Get<FVoxelMaterial>(X, Y, Z, LOD);
		}
	}
	template<bool bCustomTransform>
	TVoxelRange<v_flt> GetValueRangeImpl(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Items.IsEmpty() || WorldBounds.Intersect(LocalBounds.ApplyTransform(LocalToWorld)))
		{
			return 1;
		}
		else
		{
			const auto NextStack = Items.GetNextStack(WorldBounds);
			if (NextStack.IsValid())
			{
				return NextStack.GetValueRange(WorldBounds, LOD);
			}
			else
			{
				return { -1, 1 };
			}
		}
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface
};

/**
 * Empty world, can be used to remove
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
	void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const override
	{
		FIntBox Bounds = static_cast<const FVoxelTransformableEmptyWorldGeneratorInstance&>(Instance).LocalBounds;
		Ar << Bounds;
	}
	TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const override
	{
		FIntBox Bounds;
		Ar << Bounds;
		return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>(Bounds);
	}
	//~ End UVoxelWorldGenerator Interface
};