// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelAssets/VoxelHeightmapAssetSamplerWrapper.h"
#include "VoxelGenerators/VoxelGeneratorHelpers.h"

template<typename T>
struct TVoxelHeightmapAssetSelector;

template<>
struct TVoxelHeightmapAssetSelector<float>
{
	using Type = UVoxelHeightmapAssetFloat;
};

template<>
struct TVoxelHeightmapAssetSelector<uint16>
{
	using Type = UVoxelHeightmapAssetUINT16;
};

template<typename T>
class TVoxelHeightmapAssetInstance : public TVoxelGeneratorInstanceHelper<TVoxelHeightmapAssetInstance<T>, typename TVoxelHeightmapAssetSelector<T>::Type>
{
public:
	using Super = TVoxelGeneratorInstanceHelper<TVoxelHeightmapAssetInstance<T>, typename TVoxelHeightmapAssetSelector<T>::Type>;
	
	const TVoxelHeightmapAssetSamplerWrapper<T> Wrapper;
	const float Precision;
	const bool bInfiniteExtent;
	const FVoxelIntBox WorldBounds;

public:
	explicit TVoxelHeightmapAssetInstance(typename TVoxelHeightmapAssetSelector<T>::Type& Asset)
		: Super(&Asset)
		, Wrapper(&Asset)
		, Precision(Asset.Precision)
		, bInfiniteExtent(Asset.bInfiniteExtent)
		, WorldBounds(Asset.GetBounds())
	{
	}

	//~ Begin FVoxelGeneratorInstance Interface
	FORCEINLINE v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (bInfiniteExtent || WorldBounds.ContainsFloat(X, Y, Z)) // Note: it's safe to access outside the bounds
		{
			const float Height = Wrapper.GetHeight(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);
			return (Z - Height) / Precision;
		}
		else
		{
			// Outside asset bounds
			return 1;
		}
	}
	FORCEINLINE FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Wrapper.GetMaterial(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);
	}
	TVoxelRange<v_flt> GetValueRangeImpl(const FVoxelIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (!bInfiniteExtent && !Bounds.Intersect(WorldBounds))
		{
			return 1;
		}

		const bool bEntirelyContained = WorldBounds.Contains(Bounds);

		const auto XRange = TVoxelRange<v_flt>(Bounds.Min.X, Bounds.Max.X) + Wrapper.GetWidth() / 2;
		const auto YRange = TVoxelRange<v_flt>(Bounds.Min.Y, Bounds.Max.Y) + Wrapper.GetHeight() / 2;
		const auto ZRange = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z);

		auto HeightRange = TVoxelRange<v_flt>(Wrapper.GetHeightRange(XRange, YRange, EVoxelSamplerMode::Clamp));

		if (!bEntirelyContained && bInfiniteExtent)
		{
			// Height will be 0 outside the boundaries if bInfiniteExtent = true
			HeightRange = TVoxelRange<v_flt>::Union(HeightRange, 0.f);
		}

		auto Range = (ZRange - HeightRange) / Precision;

		if (!bEntirelyContained && !bInfiniteExtent)
		{
			// Intersects with the boundary
			Range = TVoxelRange<v_flt>::Union(1.f, Range);
		}

		return Range;
	}
	virtual void GetValues(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				const float Height = Wrapper.GetHeight(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);

				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					FVoxelValue Value;
					if (bInfiniteExtent || WorldBounds.Contains(X, Y, Z))
					{
						Value = FVoxelValue((Z - Height) / Precision);
					}
					else
					{
						// Outside asset bounds
						Value = FVoxelValue::Empty();
					}
					QueryZone.Set(X, Y, Z, Value);
				}
			}
		}
	}
	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelGeneratorInstance Interface
};