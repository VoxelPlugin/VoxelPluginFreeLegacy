// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelQueryZone.h"
#include "VoxelItemStack.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"

template<typename TVector>
FORCEINLINE FVector FVoxelGeneratorInstance::GetUpVector(const TVector& P) const
{
	return GetUpVector(P.X, P.Y, P.Z);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelGeneratorInstance::GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	if (const auto Ptr = GetOutputsPtrMap<T>().FindRef(Name))
	{
		return (this->*Ptr)(X, Y, Z, LOD, Items);
	}
	else
	{
		return DefaultValue;
	}
}
template<typename T, typename TVector>
FORCEINLINE T FVoxelGeneratorInstance::GetCustomOutput(T DefaultValue, FName Name, const TVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return GetCustomOutput<T>(DefaultValue, Name, P.X, P.Y, P.Z, LOD, Items);
}

template<typename T>
FORCEINLINE TVoxelRange<T> FVoxelGeneratorInstance::GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	if (const auto Ptr = GetOutputsRangesPtrMap<T>().FindRef(Name))
	{
		return (this->*Ptr)(Bounds, LOD, Items);
	}
	else
	{
		return DefaultValue;
	}
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelTransformableGeneratorInstance::GetCustomOutput_Transform(const FTransform& LocalToWorld, T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	if (const auto Ptr = GetOutputsPtrMap_Transform<T>().FindRef(Name))
	{
		return (this->*Ptr)(LocalToWorld, X, Y, Z, LOD, Items);
	}
	else
	{
		return DefaultValue;
	}
}

template<typename T>
FORCEINLINE TVoxelRange<T> FVoxelTransformableGeneratorInstance::GetCustomOutputRange_Transform(const FTransform& LocalToWorld, TVoxelRange<T> DefaultValue, FName Name, const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	if (const auto Ptr = GetOutputsRangesPtrMap_Transform<T>().FindRef(Name))
	{
		return (this->*Ptr)(LocalToWorld, Bounds, LOD, Items);
	}
	else
	{
		return DefaultValue;
	}
}

///////////////////////////////////////////////////////////////////////////////

FORCEINLINE v_flt FVoxelGeneratorInstance::GetValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.Value)(X, Y, Z, LOD, Items);
}

FORCEINLINE FVoxelMaterial FVoxelGeneratorInstance::GetMaterial(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.Material)(X, Y, Z, LOD, Items);
}

FORCEINLINE TVoxelRange<v_flt> FVoxelGeneratorInstance::GetValueRange(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.ValueRange)(Bounds, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

FORCEINLINE v_flt FVoxelTransformableGeneratorInstance::GetValue_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.Value)(LocalToWorld, X, Y, Z, LOD, Items);
}

FORCEINLINE FVoxelMaterial FVoxelTransformableGeneratorInstance::GetMaterial_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.Material)(LocalToWorld, X, Y, Z, LOD, Items);
}

FORCEINLINE TVoxelRange<v_flt> FVoxelTransformableGeneratorInstance::GetValueRange_Transform(const FTransform& LocalToWorld, const FVoxelIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.ValueRange)(LocalToWorld, WorldBounds, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelGeneratorInstance::Get(const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return Get<T>(P.X, P.Y, P.Z, LOD, Items);
}

template<>
FORCEINLINE FVoxelValue FVoxelGeneratorInstance::Get<FVoxelValue>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return FVoxelValue(GetValue(X, Y, Z, LOD, Items));
}
template<>
FORCEINLINE v_flt FVoxelGeneratorInstance::Get<v_flt>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetValue(X, Y, Z, LOD, Items);
}
template<>
FORCEINLINE FVoxelMaterial FVoxelGeneratorInstance::Get<FVoxelMaterial>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetMaterial(X, Y, Z, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE void FVoxelGeneratorInstance::Get<FVoxelValue>(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetValues(QueryZone, LOD, Items);
}
template<>
FORCEINLINE void FVoxelGeneratorInstance::Get<FVoxelMaterial>(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetMaterials(QueryZone, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelTransformableGeneratorInstance::Get_Transform(const FTransform& LocalToWorld, const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return Get_Transform<T>(LocalToWorld, P.X, P.Y, P.Z, LOD, Items);
}

template<>
FORCEINLINE FVoxelValue FVoxelTransformableGeneratorInstance::Get_Transform<FVoxelValue>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return FVoxelValue(GetValue_Transform(LocalToWorld, X, Y, Z, LOD, Items));
}
template<>
FORCEINLINE v_flt FVoxelTransformableGeneratorInstance::Get_Transform<v_flt>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetValue_Transform(LocalToWorld, X, Y, Z, LOD, Items);
}
template<>
FORCEINLINE FVoxelMaterial FVoxelTransformableGeneratorInstance::Get_Transform<FVoxelMaterial>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetMaterial_Transform(LocalToWorld, X, Y, Z, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE void FVoxelTransformableGeneratorInstance::Get_Transform<FVoxelValue>(
	const FTransform& LocalToWorld,
	TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetValues_Transform(LocalToWorld, QueryZone, LOD, Items);
}
template<>
FORCEINLINE void FVoxelTransformableGeneratorInstance::Get_Transform<FVoxelMaterial>(
	const FTransform& LocalToWorld,
	TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetMaterials_Transform(LocalToWorld, QueryZone, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE const TMap<FName, FVoxelGeneratorInstance::TOutputFunctionPtr<v_flt>>& FVoxelGeneratorInstance::GetOutputsPtrMap<v_flt>() const
{
	return CustomPtrs.Float;
}

template<>
FORCEINLINE const TMap<FName, FVoxelGeneratorInstance::TOutputFunctionPtr<int32>>& FVoxelGeneratorInstance::GetOutputsPtrMap<int32>() const
{
	return CustomPtrs.Int;
}

template<>
FORCEINLINE const TMap<FName, FVoxelGeneratorInstance::TOutputFunctionPtr<FColor>>& FVoxelGeneratorInstance::GetOutputsPtrMap<FColor>() const
{
	return CustomPtrs.Color;
}

template<>
FORCEINLINE const TMap<FName, FVoxelGeneratorInstance::TRangeOutputFunctionPtr<v_flt>>& FVoxelGeneratorInstance::GetOutputsRangesPtrMap<v_flt>() const
{
	return CustomPtrs.FloatRange;
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableGeneratorInstance::TOutputFunctionPtr_Transform<v_flt>>& FVoxelTransformableGeneratorInstance::GetOutputsPtrMap_Transform<v_flt>() const
{
	return CustomPtrs_Transform.Float;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableGeneratorInstance::TOutputFunctionPtr_Transform<int32>>& FVoxelTransformableGeneratorInstance::GetOutputsPtrMap_Transform<int32>() const
{
	return CustomPtrs_Transform.Int;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableGeneratorInstance::TOutputFunctionPtr_Transform<FColor>>& FVoxelTransformableGeneratorInstance::GetOutputsPtrMap_Transform<FColor>() const
{
	return CustomPtrs_Transform.Color;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableGeneratorInstance::TRangeOutputFunctionPtr_Transform<v_flt>>& FVoxelTransformableGeneratorInstance::GetOutputsRangesPtrMap_Transform<v_flt>() const
{
	return CustomPtrs_Transform.FloatRange;
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelItemStack::Get(v_flt X, v_flt Y, v_flt Z, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return Generator->Get<T>(X, Y, Z, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetAssetItems()[Depth];
		return Asset.Generator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, *this);
	}
}

FORCEINLINE TVoxelRange<v_flt> FVoxelItemStack::GetValueRange(const FVoxelIntBox& Bounds, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return Generator->GetValueRange(Bounds, LOD, *this);
	}
	else
	{
		TOptional<TVoxelRange<v_flt>> Range;

		auto& Asset = *ItemHolder.GetAssetItems()[Depth];
		if (Asset.Bounds.Intersect(Bounds))
		{
			Range = Asset.Generator->GetValueRange_Transform(
				Asset.LocalToWorld,
				Asset.Bounds.Overlap(Bounds),
				LOD,
				*this);
		}
		
		const auto NextStack = FVoxelItemStack(ItemHolder, *Generator, Depth - 1, CustomData);
		for (auto& SubBounds : Bounds.Difference(Asset.Bounds))
		{
			const auto NextRange = NextStack.GetValueRange(SubBounds, LOD);
			Range = Range.IsSet() ? TVoxelRange<v_flt>::Union(Range.GetValue(), NextRange) : NextRange;
		}

		if (!ensure(Range.IsSet()))
		{
			Range = TVoxelRange<v_flt>::Infinite();
		}
		
		return Range.GetValue();
	}
}

template<typename T>
FORCEINLINE T FVoxelItemStack::GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return Generator->GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetAssetItems()[Depth];
		return Asset.Generator->GetCustomOutput_Transform<T>(Asset.LocalToWorld, DefaultValue, Name, X, Y, Z, LOD, *this);
	}
}

template<typename T>
FORCEINLINE TVoxelRange<T> FVoxelItemStack::GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FVoxelIntBox& Bounds, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return Generator->GetCustomOutputRange<T>(DefaultValue, Name, Bounds, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetAssetItems()[Depth];
		return Asset.Generator->GetCustomOutputRange_Transform<T>(Asset.LocalToWorld, DefaultValue, Name, Bounds, LOD, *this);
	}
}

template<typename ...TArgs>
int32 FVoxelItemStack::GetNextDepth(TArgs... Args) const
{
	for (int32 Index = Depth - 1; Index >= 0; Index--)
	{
		auto& Item = *ItemHolder.GetAssetItems()[Index];
		if (Item.Bounds.ContainsTemplate(Args...))
		{
			return Index;
		}
		else if (Item.Bounds.Intersect(Args...))
		{
			// Invalid, must abort as multiple assets are possible
			return -2;
		}
	}
	return -1;
}