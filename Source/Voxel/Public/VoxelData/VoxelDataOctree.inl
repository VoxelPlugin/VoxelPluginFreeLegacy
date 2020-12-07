// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelData/VoxelDataOctree.h"
#include "VoxelGenerators/VoxelGeneratorInstance.inl"

template<typename T, typename U>
T FVoxelDataOctreeBase::GetFromGeneratorAndAssets(const FVoxelGeneratorInstance& Generator, U X, U Y, U Z, int32 LOD) const
{
	ensureThreadSafe(IsLockedForRead());
	check(IsLeafOrHasNoChildren());
	
	const auto& Assets = ItemHolder->GetAssetItems();
	for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
	{
		auto& Asset = *Assets[Index];
		if (Asset.Bounds.ContainsTemplate(X, Y, Z))
		{
			return Asset.Generator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, Generator, Index));
		}
	}
	return Generator.Get<T>(X, Y, Z, LOD, FVoxelItemStack(*ItemHolder));
}

template<typename T>
void FVoxelDataOctreeBase::GetFromGeneratorAndAssets(const FVoxelGeneratorInstance& Generator, TVoxelQueryZone<T>& QueryZone, int32 LOD) const
{
	ensureThreadSafe(IsLockedForRead());
	check(IsLeafOrHasNoChildren());
	
	const auto& Assets = ItemHolder->GetAssetItems();

	if (Assets.Num() == 0)
	{
		VOXEL_SLOW_SCOPE_COUNTER("Query Generator");
		Generator.Get(QueryZone, LOD, FVoxelItemStack(*ItemHolder));
		return;
	}

	for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
	{
		auto& Asset = *Assets[Index];
		if (Asset.Bounds.Contains(QueryZone.Bounds))
		{
			VOXEL_SLOW_SCOPE_COUNTER("Query Asset");
			Asset.Generator->Get_Transform<T>(Asset.LocalToWorld, QueryZone, LOD, FVoxelItemStack(*ItemHolder, Generator, Index));
			return;
		}
		if (Asset.Bounds.Intersect(QueryZone.Bounds))
		{
			break;
		}
	}

	VOXEL_SLOW_SCOPE_COUNTER("Individual Asset & Generator Queries");
	for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
			{
				T Value;
				for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
				{
					auto& Asset = *Assets[Index];
					if (Asset.Bounds.Contains(X, Y, Z))
					{
						Value = Asset.Generator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, Generator, Index));
						break;
					}
					if (Index == 0)
					{
						Value = Generator.Get<T>(X, Y, Z, LOD, FVoxelItemStack(*ItemHolder));
					}
				}
				QueryZone.Set(X, Y, Z, Value);
			}
		}
	}
}

template <typename T>
T FVoxelDataOctreeBase::GetCustomOutput(
	const FVoxelGeneratorInstance& Generator, 
	T DefaultValue, 
	FName Name, 
	v_flt X, v_flt Y, v_flt Z,
	int32 LOD,
	const FVoxelGeneratorQueryData& QueryData) const
{
	ensureThreadSafe(IsLockedForRead());
	check(IsLeafOrHasNoChildren());
	
	const auto& Assets = ItemHolder->GetAssetItems();
	for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
	{
		auto& Asset = *Assets[Index];
		if (Asset.Bounds.ContainsTemplate(X, Y, Z))
		{
			return Asset.Generator->GetCustomOutput_Transform(Asset.LocalToWorld, DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, Generator, Index, QueryData));
		}
	}
	return Generator.GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, QueryData));
}