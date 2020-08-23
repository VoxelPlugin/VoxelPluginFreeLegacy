// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelHardnessHandler.h"

FVoxelHardnessHandler::FVoxelHardnessHandler(const AVoxelWorld& World)
	: MaterialConfig(World.MaterialConfig)
	, RGBHardness(World.RGBHardness)
{
	for (float& It : Hardness)
	{
		It = 1;
	}
	for (auto& It : World.MaterialsHardness)
	{
		Hardness[FMath::Clamp(TCString<TCHAR>::Atoi(*It.Key), 0, 255)] = It.Value;
	}

	if (MaterialConfig == EVoxelMaterialConfig::RGB)
	{
		if (RGBHardness == EVoxelRGBHardness::FourWayBlend || RGBHardness == EVoxelRGBHardness::FiveWayBlend)
		{
			bNeedsToCompute = World.MaterialsHardness.Num() > 0;
		}
		else
		{
			bNeedsToCompute = true;
		}
	}
	else
	{
		bNeedsToCompute = World.MaterialsHardness.Num() > 0;
	}
}
