// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "VoxelMaterial.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

struct VOXEL_API FVoxelHardnessHandler
{
public:
	explicit FVoxelHardnessHandler(const AVoxelWorld& World);

	FORCEINLINE float GetHardness(const FVoxelMaterial& Material) const
	{
		return FMath::Max(GetHardnessInternal(Material), KINDA_SMALL_NUMBER);
	}
	FORCEINLINE bool NeedsToCompute() const
	{
		return bNeedsToCompute;
	}

private:
	const EVoxelMaterialConfig MaterialConfig;
	const EVoxelRGBHardness RGBHardness;
	TVoxelStaticArray<float, 256> Hardness;
	bool bNeedsToCompute;

	FORCEINLINE float GetHardnessInternal(const FVoxelMaterial& Material) const
	{
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
		{
			switch (RGBHardness)
			{
			case EVoxelRGBHardness::FourWayBlend:
			{
				const auto Strengths = FVoxelUtilities::GetFourWayBlendStrengths(Material);
				return
					Hardness[0] * Strengths[0] +
					Hardness[1] * Strengths[1] +
					Hardness[2] * Strengths[2] +
					Hardness[3] * Strengths[3];
			}
			case EVoxelRGBHardness::FiveWayBlend:
			{
				const auto Strengths = FVoxelUtilities::GetFiveWayBlendStrengths(Material);
				return
					Hardness[0] * Strengths[0] +
					Hardness[1] * Strengths[1] +
					Hardness[2] * Strengths[2] +
					Hardness[3] * Strengths[3] +
					Hardness[4] * Strengths[4];
			}
			case EVoxelRGBHardness::R: return Material.GetR_AsFloat();
			case EVoxelRGBHardness::G: return Material.GetG_AsFloat();
			case EVoxelRGBHardness::B: return Material.GetB_AsFloat();
			case EVoxelRGBHardness::A: return Material.GetA_AsFloat();
			case EVoxelRGBHardness::U0: return Material.GetU0_AsFloat();
			case EVoxelRGBHardness::U1: return Material.GetU1_AsFloat();
			case EVoxelRGBHardness::V0: return Material.GetV0_AsFloat();
			case EVoxelRGBHardness::V1: return Material.GetV1_AsFloat();
			default:
			{
				checkVoxelSlow(false);
				return 1;
			}
			}
		}
		case EVoxelMaterialConfig::SingleIndex:
		{
			return Hardness[Material.GetSingleIndex()];
		}
		case EVoxelMaterialConfig::MultiIndex:
		{
			const auto Strengths = FVoxelUtilities::GetMultiIndexStrengths(Material);
			return
					Hardness[Material.GetMultiIndex_Index0()] * Strengths[0] +
					Hardness[Material.GetMultiIndex_Index1()] * Strengths[1] +
					Hardness[Material.GetMultiIndex_Index2()] * Strengths[2] +
					Hardness[Material.GetMultiIndex_Index3()] * Strengths[3];
		}
		default:
		{
			checkVoxelSlow(false);
			return 1;
		}
		}
	}
};