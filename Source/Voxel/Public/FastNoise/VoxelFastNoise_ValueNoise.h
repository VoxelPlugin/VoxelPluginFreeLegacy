// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_ValueNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()
	GENERATED_VOXEL_NOISE_FUNCTION_2D(Value)
	GENERATED_VOXEL_NOISE_FUNCTION_2D_DERIV(Value)
	GENERATED_VOXEL_NOISE_FUNCTION_3D(Value)
	GENERATED_VOXEL_NOISE_FUNCTION_3D_DERIV(Value)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D(Value, Value)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_2D_DERIV(Value, Value)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D(Value, Value)
	GENERATED_VOXEL_NOISE_FUNCTION_FRACTAL_3D_DERIV(Value, Value)

protected:
	v_flt SingleValue_2D(uint8 offset, v_flt x, v_flt y) const;
	v_flt SingleValue_2D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt& outDx, v_flt& outDy) const;
	
	v_flt SingleValue_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const;
	v_flt SingleValue_3D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt z, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;
	
	VectorRegister SingleValue_2D(VectorRegisterInt offset, VectorRegister x, VectorRegister y) const;
	
public:
	v_flt IQNoise_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves) const;
	v_flt IQNoise_2D_Deriv(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const;

	v_flt IQNoise_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves) const;
	v_flt IQNoise_3D_Deriv(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const;
};