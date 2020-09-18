// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_PerlinNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()
	GENERATED_VOXEL_NOISE_FUNCTION_2D(Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_2D_DERIV(Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_3D(Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_3D_DERIV(Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(Perlin, Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D_DERIV(Perlin, Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(Perlin, Perlin)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D_DERIV(Perlin, Perlin)

protected:
	v_flt SinglePerlin_2D(uint8 offset, v_flt x, v_flt y) const;
	v_flt SinglePerlin_2D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt& outDx, v_flt& outDy) const;
	
	v_flt SinglePerlin_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const;
	v_flt SinglePerlin_3D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt z, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;
};