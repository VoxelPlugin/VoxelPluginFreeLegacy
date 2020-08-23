// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelHeightmapAssetData.h"
#include "VoxelAssets/VoxelHeightmapAssetData.inl"

template<typename T>
struct TVoxelHeightmapAssetSamplerWrapper
{
	const float Scale;
	const float HeightScale;
	const float HeightOffset;
	const TVoxelSharedRef<TVoxelHeightmapAssetData<T>> Data;

	explicit VOXEL_API TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset);

	float GetHeight(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * Data->GetHeight(float(X / Scale), float(Y / Scale), SamplerMode);
	}
	FVoxelMaterial GetMaterial(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return Data->GetMaterial(float(X / Scale), float(Y / Scale), SamplerMode);
	}

	TVoxelRange<float> GetHeightRange(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * TVoxelRange<float>(Data->GetHeightRange(
			{ FMath::FloorToInt(X.Min / Scale), FMath::CeilToInt(X.Max / Scale) }, 
			{ FMath::FloorToInt(Y.Min / Scale), FMath::CeilToInt(Y.Max / Scale) }, 
			SamplerMode));
	}

	void SetHeight(int32 X, int32 Y, float Height)
	{
		ensureVoxelSlowNoSideEffects(Scale == 1.f);
		Height -= HeightOffset;
		Height /= HeightScale;
		Height = FMath::Clamp<float>(Height, TNumericLimits<T>::Lowest(), TNumericLimits<T>::Max());
		if (TIsSame<T, float>::Value)
		{
			Data->SetHeight(X, Y, Height);
		}
		else
		{
			Data->SetHeight(X, Y, FMath::RoundToInt(Height));
		}
	}

	float GetMinHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMinHeight();
	}
	float GetMaxHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMaxHeight();
	}
	
	float GetWidth() const
	{
		return Scale * Data->GetWidth();
	}
	float GetHeight() const
	{
		return Scale * Data->GetHeight();
	}
};