// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGeneratorHelpers.h"

template<typename T, typename TObject = typename T::UStaticClass>
class TVoxelTransformableWorldGeneratorHelper : public TVoxelTransformableWorldGeneratorInstanceHelper<TVoxelTransformableWorldGeneratorHelper<T, TObject>, TObject>
{
public:
	const TVoxelSharedRef<T> WorldGenerator;
	const bool bSubtractiveAsset;

	explicit TVoxelTransformableWorldGeneratorHelper(
		const TVoxelSharedRef<T>& WorldGenerator,
		bool bSubtractiveAsset)
		: WorldGenerator(WorldGenerator)
		, bSubtractiveAsset(bSubtractiveAsset)
	{
	}
	
	//~ Begin FVoxelWorldGeneratorInstance Interface
	template<bool bCustomTransform>
	inline float GetValueImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		const FVector P = GetLocalPosition<bCustomTransform>(LocalToWorld, X, Y, Z);
		const v_flt Value = WorldGenerator->GetValueImpl(P.X, P.Y, P.Z, LOD, Items);
		if (Items.IsEmpty() || FVoxelValue(Value) == (bSubtractiveAsset ? FVoxelValue::Empty() : FVoxelValue::Full()))
		{
			// No need to merge as we are the best value possible
			return Value;
		}
		else
		{
			const auto NextStack = Items.GetNextStack(X, Y, Z);
			const auto NextValue = NextStack.Get<v_flt>(X, Y, Z, LOD);
			return FVoxelUtilities::MergeAsset<v_flt>(Value, NextValue, bSubtractiveAsset);
		}
	}
	
	template<bool bCustomTransform>
	inline FVoxelMaterial GetMaterialImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		const FVector P = GetLocalPosition<bCustomTransform>(LocalToWorld, X, Y, Z);
		
		const auto GetGeneratorMaterial = [&]()
		{
			return WorldGenerator->GetMaterialImpl(P.X, P.Y, P.Z, LOD, Items);
		};

		if (Items.IsEmpty())
		{
			return GetGeneratorMaterial();
		}

		const FVoxelValue Value = FVoxelValue(WorldGenerator->GetValueImpl(P.X, P.Y, P.Z, LOD, Items));
		if ((bSubtractiveAsset && Value == FVoxelValue::Empty()) ||
			(!bSubtractiveAsset && Value == FVoxelValue::Full()))
		{
			// No need to check further down
			return GetGeneratorMaterial();
		}

		const auto NextStack = Items.GetNextStack(X, Y, Z);
		const auto NextValue = NextStack.Get<FVoxelValue>(X, Y, Z, LOD);
		if (bSubtractiveAsset ? Value >= NextValue : Value <= NextValue)
		{
			// We have a better value
			return GetGeneratorMaterial();
		}
		else
		{
			return NextStack.Get<FVoxelMaterial>(X, Y, Z, LOD);
		}
	}
	
	template<bool bCustomTransform>
	TVoxelRange<v_flt> GetValueRangeImpl(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		const FIntBox LocalBounds = bCustomTransform ? WorldBounds.ApplyTransform<EInverseTransform::True>(LocalToWorld) : WorldBounds;
		const TVoxelRange<v_flt> WorldGeneratorRange = WorldGenerator->GetValueRangeImpl(LocalBounds, LOD, Items);

		const auto GetNextRange = [&]() -> TVoxelRange<v_flt>
		{
			if (Items.IsEmpty())
			{
				return bSubtractiveAsset ? -1 : 1;
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
					return TVoxelRange<v_flt>::Infinite();
				}
			}
		};
		const TVoxelRange<v_flt> NextRange = GetNextRange();

		return bSubtractiveAsset
			? FVoxelRangeUtilities::Max(WorldGeneratorRange, NextRange)
			: FVoxelRangeUtilities::Min(WorldGeneratorRange, NextRange);
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return WorldGenerator->GetUpVector(X, Y, Z);
	}
	//~ End FVoxelWorldGeneratorInstance Interface

private:
	template<bool bCustomTransform>
	FORCEINLINE FVector GetLocalPosition(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z) const
	{
		if (bCustomTransform)
		{
			const auto LocalPosition = LocalToWorld.InverseTransformPosition(FVector(X, Y, Z));
			X = LocalPosition.X;
			Y = LocalPosition.Y;
			Z = LocalPosition.Z;
		}
		return FVector(X, Y, Z);
	}
};