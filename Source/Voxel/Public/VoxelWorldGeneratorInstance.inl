// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelPlaceableItems/VoxelAssetItem.h"
#include "VoxelWorldGeneratorInstance.h"

template<typename TVector>
FORCEINLINE FVector FVoxelWorldGeneratorInstance::GetUpVector(const TVector& P) const
{
	return GetUpVector(P.X, P.Y, P.Z);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelWorldGeneratorInstance::GetCustomOutput(T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
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
FORCEINLINE T FVoxelWorldGeneratorInstance::GetCustomOutput(T DefaultValue, FName Name, const TVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return GetCustomOutput<T>(DefaultValue, Name, P.X, P.Y, P.Z, LOD, Items);
}

template<typename T>
FORCEINLINE TVoxelRange<T> FVoxelWorldGeneratorInstance::GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
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
FORCEINLINE T FVoxelTransformableWorldGeneratorInstance::GetCustomOutput_Transform(const FTransform& LocalToWorld, T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
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
FORCEINLINE TVoxelRange<T> FVoxelTransformableWorldGeneratorInstance::GetCustomOutputRange_Transform(const FTransform& LocalToWorld, TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
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

FORCEINLINE v_flt FVoxelWorldGeneratorInstance::GetValue(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.Value)(X, Y, Z, LOD, Items);
}

FORCEINLINE FVoxelMaterial FVoxelWorldGeneratorInstance::GetMaterial(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.Material)(X, Y, Z, LOD, Items);
}

FORCEINLINE TVoxelRange<v_flt> FVoxelWorldGeneratorInstance::GetValueRange(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs.ValueRange)(Bounds, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

FORCEINLINE v_flt FVoxelTransformableWorldGeneratorInstance::GetValue_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.Value)(LocalToWorld, X, Y, Z, LOD, Items);
}

FORCEINLINE FVoxelMaterial FVoxelTransformableWorldGeneratorInstance::GetMaterial_Transform(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.Material)(LocalToWorld, X, Y, Z, LOD, Items);
}

FORCEINLINE TVoxelRange<v_flt> FVoxelTransformableWorldGeneratorInstance::GetValueRange_Transform(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
{
	return (this->*BasePtrs_Transform.ValueRange)(LocalToWorld, WorldBounds, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelWorldGeneratorInstance::Get(const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return Get<T>(P.X, P.Y, P.Z, LOD, Items);
}

template<>
FORCEINLINE FVoxelValue FVoxelWorldGeneratorInstance::Get<FVoxelValue>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return FVoxelValue(GetValue(X, Y, Z, LOD, Items));
}
template<>
FORCEINLINE v_flt FVoxelWorldGeneratorInstance::Get<v_flt>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetValue(X, Y, Z, LOD, Items);
}
template<>
FORCEINLINE FVoxelMaterial FVoxelWorldGeneratorInstance::Get<FVoxelMaterial>(
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetMaterial(X, Y, Z, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE void FVoxelWorldGeneratorInstance::Get<FVoxelValue>(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetValues(QueryZone, LOD, Items);
}
template<>
FORCEINLINE void FVoxelWorldGeneratorInstance::Get<FVoxelMaterial>(TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetMaterials(QueryZone, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE T FVoxelTransformableWorldGeneratorInstance::Get_Transform(const FTransform& LocalToWorld, const FIntVector& P, int32 LOD, const FVoxelItemStack& Items) const
{
	return Get_Transform<T>(LocalToWorld, P.X, P.Y, P.Z, LOD, Items);
}

template<>
FORCEINLINE FVoxelValue FVoxelTransformableWorldGeneratorInstance::Get_Transform<FVoxelValue>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return FVoxelValue(GetValue_Transform(LocalToWorld, X, Y, Z, LOD, Items));
}
template<>
FORCEINLINE v_flt FVoxelTransformableWorldGeneratorInstance::Get_Transform<v_flt>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetValue_Transform(LocalToWorld, X, Y, Z, LOD, Items);
}
template<>
FORCEINLINE FVoxelMaterial FVoxelTransformableWorldGeneratorInstance::Get_Transform<FVoxelMaterial>(
	const FTransform& LocalToWorld,
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD, const FVoxelItemStack& Items) const
{
	return GetMaterial_Transform(LocalToWorld, X, Y, Z, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE void FVoxelTransformableWorldGeneratorInstance::Get_Transform<FVoxelValue>(
	const FTransform& LocalToWorld,
	TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetValues_Transform(LocalToWorld, QueryZone, LOD, Items);
}
template<>
FORCEINLINE void FVoxelTransformableWorldGeneratorInstance::Get_Transform<FVoxelMaterial>(
	const FTransform& LocalToWorld,
	TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const
{
	GetMaterials_Transform(LocalToWorld, QueryZone, LOD, Items);
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE const TMap<FName, FVoxelWorldGeneratorInstance::TOutputFunctionPtr<v_flt>>& FVoxelWorldGeneratorInstance::GetOutputsPtrMap<v_flt>() const
{
	return CustomPtrs.Float;
}

template<>
FORCEINLINE const TMap<FName, FVoxelWorldGeneratorInstance::TOutputFunctionPtr<int32>>& FVoxelWorldGeneratorInstance::GetOutputsPtrMap<int32>() const
{
	return CustomPtrs.Int;
}

template<>
FORCEINLINE const TMap<FName, FVoxelWorldGeneratorInstance::TOutputFunctionPtr<FColor>>& FVoxelWorldGeneratorInstance::GetOutputsPtrMap<FColor>() const
{
	return CustomPtrs.Color;
}

template<>
FORCEINLINE const TMap<FName, FVoxelWorldGeneratorInstance::TRangeOutputFunctionPtr<v_flt>>& FVoxelWorldGeneratorInstance::GetOutputsRangesPtrMap<v_flt>() const
{
	return CustomPtrs.FloatRange;
}

///////////////////////////////////////////////////////////////////////////////

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableWorldGeneratorInstance::TOutputFunctionPtr_Transform<v_flt>>& FVoxelTransformableWorldGeneratorInstance::GetOutputsPtrMap_Transform<v_flt>() const
{
	return CustomPtrs_Transform.Float;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableWorldGeneratorInstance::TOutputFunctionPtr_Transform<int32>>& FVoxelTransformableWorldGeneratorInstance::GetOutputsPtrMap_Transform<int32>() const
{
	return CustomPtrs_Transform.Int;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableWorldGeneratorInstance::TOutputFunctionPtr_Transform<FColor>>& FVoxelTransformableWorldGeneratorInstance::GetOutputsPtrMap_Transform<FColor>() const
{
	return CustomPtrs_Transform.Color;
}

template<>
FORCEINLINE const TMap<FName, FVoxelTransformableWorldGeneratorInstance::TRangeOutputFunctionPtr_Transform<v_flt>>& FVoxelTransformableWorldGeneratorInstance::GetOutputsRangesPtrMap_Transform<v_flt>() const
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
		return WorldGenerator->Get<T>(X, Y, Z, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetItems<FVoxelAssetItem>()[Depth];
		return Asset.WorldGenerator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, *this);
	}
}

FORCEINLINE TVoxelRange<v_flt> FVoxelItemStack::GetValueRange(const FIntBox& Bounds, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return WorldGenerator->GetValueRange(Bounds, LOD, *this);
	}
	else
	{
		TOptional<TVoxelRange<v_flt>> Range;

		auto& Asset = *ItemHolder.GetItems<FVoxelAssetItem>()[Depth];
		if (Asset.Bounds.Intersect(Bounds))
		{
			Range = Asset.WorldGenerator->GetValueRange_Transform(
				Asset.LocalToWorld,
				Asset.Bounds.Overlap(Bounds),
				LOD,
				*this);
		}
		
		const auto NextStack = FVoxelItemStack(ItemHolder, *WorldGenerator, Depth - 1, CustomData);
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
		return WorldGenerator->GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetItems<FVoxelAssetItem>()[Depth];
		return Asset.WorldGenerator->GetCustomOutput_Transform<T>(Asset.LocalToWorld, DefaultValue, Name, X, Y, Z, LOD, *this);
	}
}

template<typename T>
FORCEINLINE TVoxelRange<T> FVoxelItemStack::GetCustomOutputRange(TVoxelRange<T> DefaultValue, FName Name, const FIntBox& Bounds, int32 LOD) const
{
	check(IsValid());
	if (Depth == -1)
	{
		return WorldGenerator->GetCustomOutputRange<T>(DefaultValue, Name, Bounds, LOD, *this);
	}
	else
	{
		auto& Asset = *ItemHolder.GetItems<FVoxelAssetItem>()[Depth];
		return Asset.WorldGenerator->GetCustomOutputRange_Transform<T>(Asset.LocalToWorld, DefaultValue, Name, Bounds, LOD, *this);
	}
}

template<typename ...TArgs>
int32 FVoxelItemStack::GetNextDepth(TArgs... Args) const
{
	for (int32 Index = Depth - 1; Index >= 0; Index--)
	{
		auto& Item = *ItemHolder.GetItems<FVoxelAssetItem>()[Index];
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