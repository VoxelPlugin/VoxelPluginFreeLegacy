// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataOctree.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelWorldGeneratorInstance.inl"

DEFINE_STAT(STAT_VoxelDataOctreesMemory);
DEFINE_STAT(STAT_VoxelDataOctreesCount);

DEFINE_STAT(STAT_VoxelDataOctreeValuesMemory);
DEFINE_STAT(STAT_VoxelDataOctreeMaterialsMemory);
DEFINE_STAT(STAT_VoxelDataOctreeFoliageMemory);

template<typename T, typename U>
T FVoxelDataOctreeBase::GetFromGeneratorAndAssets(const FVoxelWorldGeneratorInstance& WorldGenerator, U X, U Y, U Z, int32 LOD) const
{
	const auto Assets = ItemHolder->GetItems<FVoxelAssetItem>();
	if (Assets.Num() > 0)
	{
		for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
		{
			auto& Asset = *Assets[Index];
			if (Asset.Bounds.ContainsTemplate(X, Y, Z))
			{
				return Asset.WorldGenerator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, WorldGenerator, Index));
			}
		}
	}
	return WorldGenerator.Get<T>(X, Y, Z, LOD, FVoxelItemStack(*ItemHolder));
}

template VOXEL_API v_flt          FVoxelDataOctreeBase::GetFromGeneratorAndAssets<v_flt         , v_flt>(const FVoxelWorldGeneratorInstance& WorldGenerator, v_flt X, v_flt Y, v_flt Z, int32 LOD) const;
template VOXEL_API v_flt          FVoxelDataOctreeBase::GetFromGeneratorAndAssets<v_flt         , int32>(const FVoxelWorldGeneratorInstance& WorldGenerator, int32 X, int32 Y, int32 Z, int32 LOD) const;
template VOXEL_API FVoxelValue    FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelValue   , int32>(const FVoxelWorldGeneratorInstance& WorldGenerator, int32 X, int32 Y, int32 Z, int32 LOD) const;
template VOXEL_API FVoxelMaterial FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelMaterial, int32>(const FVoxelWorldGeneratorInstance& WorldGenerator, int32 X, int32 Y, int32 Z, int32 LOD) const;

template<typename T>
void FVoxelDataOctreeBase::GetFromGeneratorAndAssets(const FVoxelWorldGeneratorInstance& WorldGenerator, TVoxelQueryZone<T>& QueryZone, int32 LOD) const
{
	const auto Assets = ItemHolder->GetItems<FVoxelAssetItem>();

	if (Assets.Num() == 0)
	{
		VOXEL_SLOW_SCOPE_COUNTER("Query World Generator");
		WorldGenerator.Get(QueryZone, LOD, FVoxelItemStack(*ItemHolder));
		return;
	}

	for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
	{
		auto& Asset = *Assets[Index];
		if (QueryZone.Bounds.Contains(Asset.Bounds))
		{
			VOXEL_SLOW_SCOPE_COUNTER("Query Asset");
			Asset.WorldGenerator->Get_Transform<T>(Asset.LocalToWorld, QueryZone, LOD, FVoxelItemStack(*ItemHolder, WorldGenerator, Index));
			return;
		}
		if (QueryZone.Bounds.Intersect(Asset.Bounds))
		{
			break;
		}
	}

	VOXEL_SLOW_SCOPE_COUNTER("Individual Asset & World Generator Queries");
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
						Value = Asset.WorldGenerator->Get_Transform<T>(Asset.LocalToWorld, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, WorldGenerator, Index));
						break;
					}
					if (Index == 0)
					{
						Value = WorldGenerator.Get<T>(X, Y, Z, LOD, FVoxelItemStack(*ItemHolder));
					}
				}
				QueryZone.Set(X, Y, Z, Value);
			}
		}
	}
}

template VOXEL_API void FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelValue   >(const FVoxelWorldGeneratorInstance& WorldGenerator, TVoxelQueryZone<FVoxelValue   >& QueryZone, int32 LOD) const;
template VOXEL_API void FVoxelDataOctreeBase::GetFromGeneratorAndAssets<FVoxelMaterial>(const FVoxelWorldGeneratorInstance& WorldGenerator, TVoxelQueryZone<FVoxelMaterial>& QueryZone, int32 LOD) const;

template <typename T>
T FVoxelDataOctreeBase::GetCustomOutput(const FVoxelWorldGeneratorInstance& WorldGenerator, T DefaultValue, FName Name, v_flt X, v_flt Y, v_flt Z, int32 LOD) const
{
	check(IsLeafOrHasNoChildren());
	ensureThreadSafe(IsLockedForRead());
	const auto Assets = ItemHolder->GetItems<FVoxelAssetItem>();
	if (Assets.Num() > 0)
	{
		for (int32 Index = Assets.Num() - 1; Index >= 0; Index--)
		{
			auto& Asset = *Assets[Index];
			if (Asset.Bounds.ContainsTemplate(X, Y, Z))
			{
				return Asset.WorldGenerator->GetCustomOutput_Transform(Asset.LocalToWorld, DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder, WorldGenerator, Index));
			}
		}
	}
	return WorldGenerator.GetCustomOutput<T>(DefaultValue, Name, X, Y, Z, LOD, FVoxelItemStack(*ItemHolder));
}

template VOXEL_API v_flt FVoxelDataOctreeBase::GetCustomOutput<v_flt>(const FVoxelWorldGeneratorInstance&, v_flt, FName, v_flt, v_flt, v_flt, int32) const;
template VOXEL_API int32 FVoxelDataOctreeBase::GetCustomOutput<int32>(const FVoxelWorldGeneratorInstance&, int32, FName, v_flt, v_flt, v_flt, int32) const;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctreeParent::CreateChildren()
{
	TVoxelOctreeParent::CreateChildren();

#if DO_THREADSAFE_CHECKS
	for (auto& Child : AsParent().GetChildren())
	{
		Child.Parent = this;
	}
#endif

	const auto& AllItems = ItemHolder->GetAllItems();
	if (AllItems.Num() > 0)
	{
		for (auto& Child : AsParent().GetChildren())
		{
			const FIntBox ChildBounds = Child.GetBounds();
			for (auto& Items : AllItems)
			{
				for (auto* Item : Items)
				{
					if (Item->Bounds.Intersect(ChildBounds))
					{
						Child.ItemHolder->AddItem(Item);
					}
				}
			}
		}
	}
	ItemHolder.Reset();
}

void FVoxelDataOctreeParent::DestroyChildren()
{
	TVoxelOctreeParent::DestroyChildren();

	check(!ItemHolder.IsValid());
	// Always valid on a node with no children
	ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>();
}