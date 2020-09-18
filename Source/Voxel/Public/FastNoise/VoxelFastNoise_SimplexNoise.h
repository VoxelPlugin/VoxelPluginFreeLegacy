// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_SimplexNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()
	GENERATED_VOXEL_NOISE_FUNCTION_2D(Simplex)
	GENERATED_VOXEL_NOISE_FUNCTION_3D(Simplex)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(Simplex, Simplex)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(Simplex, Simplex)

protected:
	static constexpr v_flt SQRT3 = v_flt(1.7320508075688772935274463415059);
	static constexpr v_flt F2 = v_flt(0.5) * (SQRT3 - v_flt(1.0));
	static constexpr v_flt G2 = (v_flt(3.0) - SQRT3) / v_flt(6.0);
	
	static constexpr v_flt F3 = 1 / v_flt(3);
	static constexpr v_flt G3 = 1 / v_flt(6);
	
	v_flt SingleSimplex_2D(uint8 offset, v_flt x, v_flt y) const;
	v_flt SingleSimplex_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const;
};