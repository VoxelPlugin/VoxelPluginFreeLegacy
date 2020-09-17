// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFastNoiseBase.h"

template<typename T>
class TVoxelFastNoise_GradientPerturb : public T
{
public:
	DEFINE_VOXEL_NOISE_CLASS()

	void GradientPerturb_2D(v_flt& x, v_flt& y, v_flt frequency, v_flt m_gradientPerturbAmp) const;
	void GradientPerturb_3D(v_flt& x, v_flt& y, v_flt& z, v_flt frequency, v_flt m_gradientPerturbAmp) const;

	void GradientPerturbFractal_2D(v_flt& x, v_flt& y, v_flt frequency, int32 octaves, v_flt m_gradientPerturbAmp) const;
	void GradientPerturbFractal_3D(v_flt& x, v_flt& y, v_flt& z, v_flt frequency, int32 octaves, v_flt m_gradientPerturbAmp) const;

protected:
	void SingleGradientPerturb_2D(uint8 offset, v_flt warpAmp, v_flt frequency, v_flt& x, v_flt& y) const;
	void SingleGradientPerturb_3D(uint8 offset, v_flt warpAmp, v_flt frequency, v_flt& x, v_flt& y, v_flt& z) const;
};