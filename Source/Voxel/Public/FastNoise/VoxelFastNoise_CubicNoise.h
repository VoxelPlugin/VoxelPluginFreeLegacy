// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_CubicNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()
	GENERATED_VOXEL_NOISE_FUNCTION_2D(Cubic)
	GENERATED_VOXEL_NOISE_FUNCTION_3D(Cubic)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(Cubic, Cubic)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(Cubic, Cubic)

protected:
	static constexpr v_flt CUBIC_2D_BOUNDING = 1 / (v_flt(1.5) * v_flt(1.5));
	static constexpr v_flt CUBIC_3D_BOUNDING = 1 / (v_flt(1.5) * v_flt(1.5) * v_flt(1.5));

	v_flt SingleCubic_2D(uint8 offset, v_flt x, v_flt y) const;
	v_flt SingleCubic_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const;
};