// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelTools/VoxelToolManager.h"
#include "VoxelTools/VoxelSurfaceTools.h"

namespace FVoxelToolManagerToolsHelpers
{
	inline FVoxelSurfaceEditsStackElement GetApplyFalloff(
		EVoxelToolManagerFalloff FalloffType, 
		AVoxelWorld& World,
		const FVector& ToolPosition,
		float Radius, 
		float FalloffBetween0And1)
	{
		const float RadiusWithoutFalloff = Radius * (1 - FalloffBetween0And1);
		const float Falloff = Radius * FalloffBetween0And1;
		switch (FalloffType)
		{
		default: ensure(false);
		case EVoxelToolManagerFalloff::Linear:
			return UVoxelSurfaceTools::ApplyLinearFalloff(&World, ToolPosition, RadiusWithoutFalloff, Falloff);
		case EVoxelToolManagerFalloff::Smooth:
			return UVoxelSurfaceTools::ApplySmoothFalloff(&World, ToolPosition, RadiusWithoutFalloff, Falloff);
		case EVoxelToolManagerFalloff::Spherical:
			return UVoxelSurfaceTools::ApplySphericalFalloff(&World, ToolPosition, RadiusWithoutFalloff, Falloff);
		case EVoxelToolManagerFalloff::Tip:
			return UVoxelSurfaceTools::ApplyTipFalloff(&World, ToolPosition, RadiusWithoutFalloff, Falloff);
		}
	}
}