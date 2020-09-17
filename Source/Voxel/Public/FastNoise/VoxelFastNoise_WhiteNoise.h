// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_WhiteNoise : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()

	v_flt GetWhiteNoise_2D(v_flt x, v_flt y) const;
	v_flt GetWhiteNoiseInt_2D(int32 x, int32 y) const;

	v_flt GetWhiteNoise_3D(v_flt x, v_flt y, v_flt z) const;
	v_flt GetWhiteNoiseInt_3D(int32 x, int32 y, int32 z) const;
	
	v_flt GetWhiteNoise_4D(v_flt x, v_flt y, v_flt z, v_flt w) const;
	v_flt GetWhiteNoiseInt_4D(int32 x, int32 y, int32 z, int32 w) const;
};