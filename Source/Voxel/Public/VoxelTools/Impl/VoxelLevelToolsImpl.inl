// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelTools/Impl/VoxelLevelToolsImpl.h"
#include "VoxelTools/Impl/VoxelToolsBaseImpl.inl"

inline FVoxelIntBox FVoxelLevelToolsImpl::GetBounds(const FVoxelVector& Position, float Radius, float Height, bool bAdditive)
{
	const float R = Radius + 2;
	if (bAdditive)
	{
		// Below
		return FVoxelIntBox(Position - FVector(R, R, Height + 1.f), Position + FVector(R, R, 1.f));
	}
	else
	{
		// Above
		return FVoxelIntBox(Position - FVector(R, R, 1.f), Position + FVector(R, R, Height + 1.f));
	}
}

template<typename TData>
void FVoxelLevelToolsImpl::Level(TData& Data, const FVoxelVector& Position, float Radius, float Falloff, float Height, bool bAdditive)
{
	const FVoxelIntBox Bounds = GetBounds(Position, Radius, Height, bAdditive);
	VOXEL_TOOL_FUNCTION_COUNTER(Bounds.Count());

	// Center the height
	const FVoxelVector CylinderPosition = Position + FVoxelVector(0, 0, (bAdditive ? -Height : Height) / 2);
	
	const float SquaredRadius = FMath::Square(Radius + 2);
	Data.template Set<FVoxelValue>(Bounds, [&](int32 X, int32 Y, int32 Z, FVoxelValue& Value)
	{
		const float SquaredDistance = FVector2D(X - Position.X, Y - Position.Y).SizeSquared();
		if (SquaredDistance <= SquaredRadius)
		{
			const float SDF = FVoxelUtilities::RoundCylinder(FVoxelVector(X, Y, Z) - CylinderPosition, Radius, Height, Falloff);
			if (bAdditive)
			{
				Value = FMath::Min(Value, FVoxelValue(SDF));
			}
			else
			{
				Value = FMath::Max(Value, FVoxelValue(-SDF));
			}
		}
	});
}