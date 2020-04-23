// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManager.h"

namespace FVoxelToolManagerToolsHelpers
{
	template<typename T>
	inline void DispatchApplyFalloff(EVoxelToolManagerFalloff FalloffType, float RadiusInVoxels, float FalloffBetween0And1, T ApplyFalloff)
	{
		const float RadiusWithoutFalloff = RadiusInVoxels * (1 - FalloffBetween0And1);
		const float Falloff = RadiusInVoxels * FalloffBetween0And1;
		switch (FalloffType)
		{
		case EVoxelToolManagerFalloff::Linear:
			ApplyFalloff([=](float Distance) { return FVoxelUtilities::LinearFalloff(Distance, RadiusWithoutFalloff, Falloff); });
			break;
		case EVoxelToolManagerFalloff::Smooth:
			ApplyFalloff([=](float Distance) { return FVoxelUtilities::SmoothFalloff(Distance, RadiusWithoutFalloff, Falloff); });
			break;
		case EVoxelToolManagerFalloff::Spherical:
			ApplyFalloff([=](float Distance) { return FVoxelUtilities::SphericalFalloff(Distance, RadiusWithoutFalloff, Falloff); });
			break;
		case EVoxelToolManagerFalloff::Tip:
			ApplyFalloff([=](float Distance) { return FVoxelUtilities::TipFalloff(Distance, RadiusWithoutFalloff, Falloff); });
			break;
		default: ensure(false);
		}
	}
}