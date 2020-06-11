// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelMathUtilities.h"

class UMaterialInterface;

namespace FVoxelUtilities
{
	VOXEL_API bool IsMaterialTessellated(UMaterialInterface* Material);

	// NumIndices: else chunks get merged with different number of tex coordinates
	VOXEL_API UMaterialInterface* GetDefaultMaterial(int32 NumIndices);
	
	FORCEINLINE int32 GetMultiIndexIndex(const FVoxelMaterial& Material, int32 Channel)
	{
		if (Channel == Material.GetMultiIndex_Index0())
		{
			return 0;
		}
		else if (Channel == Material.GetMultiIndex_Index1())
		{
			return 1;
		}
		else if (Channel == Material.GetMultiIndex_Index2())
		{
			return 2;
		}
		else if (Channel == Material.GetMultiIndex_Index3())
		{
			return 3;
		}
		else
		{
			return -1;
		}
	}

	FORCEINLINE TVoxelStaticArray<float, 4> GetMultiIndexStrengths(const FVoxelMaterial& Material)
	{
		return FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>(
		{
			Material.GetMultiIndex_Blend0_AsFloat(),
			Material.GetMultiIndex_Blend1_AsFloat(),
			Material.GetMultiIndex_Blend2_AsFloat()
		});
	}
	FORCEINLINE TVoxelStaticArray<float, 5> GetFiveWayBlendStrengths(const FVoxelMaterial& Material)
	{
		return FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<5>(
		{
			Material.GetR_AsFloat(),
			Material.GetG_AsFloat(),
			Material.GetB_AsFloat(),
			Material.GetA_AsFloat()
		});
	}
	FORCEINLINE TVoxelStaticArray<float, 4> GetFourWayBlendStrengths(const FVoxelMaterial& Material)
	{
		return FVoxelUtilities::XWayBlend_AlphasToStrengths_Static<4>(
		{
			Material.GetR_AsFloat(),
			Material.GetG_AsFloat(),
			Material.GetB_AsFloat()
		});
	}
}
