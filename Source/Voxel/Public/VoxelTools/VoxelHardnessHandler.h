// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorld.h"
#include "VoxelMaterial.h"
#include "VoxelBaseUtilities.h"
#include "StackArray.h"

struct VOXEL_API FVoxelHardnessHandler
{
public:
	explicit FVoxelHardnessHandler(const AVoxelWorld& World)
		: MaterialConfig(World.MaterialConfig)
		, bUseAlphaAsHardness(World.bUseAlphaAsHardness)
		, bDefaultValues(MaterialConfig == EVoxelMaterialConfig::RGB ? !bUseAlphaAsHardness : World.MaterialsHardness.Num() == 0)
	{
		auto& Array = const_cast<TStackArray<float, 256>&>(Hardness);
		for (float& It : Array)
		{
			It = 1;
		}
		for (auto& It : World.MaterialsHardness)
		{
			Array[FMath::Clamp(TCString<TCHAR>::Atoi(*It.Key), 0, 255)] = It.Value;
		}
	}
	inline float GetHardness(const FVoxelMaterial& Material) const
	{
		return FMath::Max(GetHardnessInternal(Material), KINDA_SMALL_NUMBER);
	}
	FORCEINLINE bool NeedsToCompute() const
	{
		return !bDefaultValues;
	}

private:
	const EVoxelMaterialConfig MaterialConfig;
	const bool bUseAlphaAsHardness;
	const TStackArray<float, 256> Hardness;
	const bool bDefaultValues;

	float GetHardnessInternal(const FVoxelMaterial& Material) const
	{
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
			return bUseAlphaAsHardness ? FVoxelUtilities::UINT8ToFloat(Material.GetA()) : 1;
		case EVoxelMaterialConfig::SingleIndex:
			return Hardness[Material.GetSingleIndex_Index()];
		case EVoxelMaterialConfig::DoubleIndex:
			return FMath::Lerp(
				Hardness[Material.GetDoubleIndex_IndexA()],
				Hardness[Material.GetDoubleIndex_IndexB()],
				Material.GetDoubleIndex_Blend_AsFloat());
		default:
			checkVoxelSlow(false);
			return 1;
		}
	}
};