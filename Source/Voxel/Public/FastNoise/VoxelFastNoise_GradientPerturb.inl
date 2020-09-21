// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_GradientPerturb.h"
#include "FastNoise/VoxelFastNoiseBase.inl"

// THIS CODE IS A MODIFIED VERSION OF FAST NOISE: SEE LICENSE BELOW
//
// MIT License
//
// Copyright(c) 2017 Jordan Peck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The developer's email is jorzixdan.me2@gzixmail.com (for great email, take
// off every 'zix'.)
//
// VERSION: 0.4.1

// ReSharper disable CppUE4CodingStandardNamingViolationWarning


template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_GradientPerturb<T>::GradientPerturb_2D(v_flt& x, v_flt& y, v_flt frequency, v_flt m_gradientPerturbAmp) const
{
	SingleGradientPerturb_2D(0, m_gradientPerturbAmp, frequency, x, y);
}

template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_GradientPerturb<T>::GradientPerturb_3D(v_flt& x, v_flt& y, v_flt& z, v_flt frequency, v_flt m_gradientPerturbAmp) const
{
	SingleGradientPerturb_3D(0, m_gradientPerturbAmp, frequency, x, y, z);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_GradientPerturb<T>::GradientPerturbFractal_2D(v_flt& x, v_flt& y, v_flt frequency, int32 octaves, v_flt m_gradientPerturbAmp) const
{
	v_flt amp = m_gradientPerturbAmp * This().FractalBounding;
	v_flt freq = frequency;
	int32 i = 0;

	SingleGradientPerturb_2D(This().Perm[0], amp, frequency, x, y);

	while (++i < octaves)
	{
		freq *= This().Lacunarity;
		amp *= This().Gain;
		SingleGradientPerturb_2D(This().Perm[i], amp, freq, x, y);
	}
}

template<typename T>
FN_FORCEINLINE void TVoxelFastNoise_GradientPerturb<T>::GradientPerturbFractal_3D(v_flt& x, v_flt& y, v_flt& z, v_flt frequency, int32 octaves, v_flt m_gradientPerturbAmp) const
{
	v_flt amp = m_gradientPerturbAmp * This().FractalBounding;
	v_flt freq = frequency;
	int32 i = 0;

	SingleGradientPerturb_3D(This().Perm[0], amp, frequency, x, y, z);

	while (++i < octaves)
	{
		freq *= This().Lacunarity;
		amp *= This().Gain;
		SingleGradientPerturb_3D(This().Perm[i], amp, freq, x, y, z);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_SINGLE void TVoxelFastNoise_GradientPerturb<T>::SingleGradientPerturb_2D(uint8 offset, v_flt warpAmp, v_flt frequency, v_flt& x, v_flt& y) const
{
	const v_flt xf = x * frequency;
	const v_flt yf = y * frequency;

	const int32 x0 = FNoiseMath::FastFloor(xf);
	const int32 y0 = FNoiseMath::FastFloor(yf);

	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;

	const v_flt fx = xf - x0;
	const v_flt fy = yf - y0;

	v_flt xs, ys;
	This().Interpolate_2D(fx, fy, xs, ys);

	int32 lutPos0 = This().Index2D_256(offset, x0, y0);
	int32 lutPos1 = This().Index2D_256(offset, x1, y0);

	const v_flt lx0x = FNoiseMath::Lerp(This().CELL_2D_X[lutPos0], This().CELL_2D_X[lutPos1], xs);
	const v_flt ly0x = FNoiseMath::Lerp(This().CELL_2D_Y[lutPos0], This().CELL_2D_Y[lutPos1], xs);

	lutPos0 = This().Index2D_256(offset, x0, y1);
	lutPos1 = This().Index2D_256(offset, x1, y1);

	const v_flt lx1x = FNoiseMath::Lerp(This().CELL_2D_X[lutPos0], This().CELL_2D_X[lutPos1], xs);
	const v_flt ly1x = FNoiseMath::Lerp(This().CELL_2D_Y[lutPos0], This().CELL_2D_Y[lutPos1], xs);

	x += FNoiseMath::Lerp(lx0x, lx1x, ys) * warpAmp;
	y += FNoiseMath::Lerp(ly0x, ly1x, ys) * warpAmp;
}

template<typename T>
FN_FORCEINLINE_SINGLE void TVoxelFastNoise_GradientPerturb<T>::SingleGradientPerturb_3D(uint8 offset, v_flt warpAmp, v_flt frequency, v_flt& x, v_flt& y, v_flt& z) const
{
	const v_flt xf = x * frequency;
	const v_flt yf = y * frequency;
	const v_flt zf = z * frequency;

	const int32 x0 = FNoiseMath::FastFloor(xf);
	const int32 y0 = FNoiseMath::FastFloor(yf);
	const int32 z0 = FNoiseMath::FastFloor(zf);

	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;
	const int32 z1 = z0 + 1;

	const v_flt fx = xf - x0;
	const v_flt fy = yf - y0;
	const v_flt fz = zf - z0;

	v_flt xs, ys, zs;
	This().Interpolate_3D(fx, fy, fz, xs, ys, zs);

	int32 lutPos0 = This().Index3D_256(offset, x0, y0, z0);
	int32 lutPos1 = This().Index3D_256(offset, x1, y0, z0);

	v_flt lx0x = FNoiseMath::Lerp(This().CELL_3D_X[lutPos0], This().CELL_3D_X[lutPos1], xs);
	v_flt ly0x = FNoiseMath::Lerp(This().CELL_3D_Y[lutPos0], This().CELL_3D_Y[lutPos1], xs);
	v_flt lz0x = FNoiseMath::Lerp(This().CELL_3D_Z[lutPos0], This().CELL_3D_Z[lutPos1], xs);

	lutPos0 = This().Index3D_256(offset, x0, y1, z0);
	lutPos1 = This().Index3D_256(offset, x1, y1, z0);

	v_flt lx1x = FNoiseMath::Lerp(This().CELL_3D_X[lutPos0], This().CELL_3D_X[lutPos1], xs);
	v_flt ly1x = FNoiseMath::Lerp(This().CELL_3D_Y[lutPos0], This().CELL_3D_Y[lutPos1], xs);
	v_flt lz1x = FNoiseMath::Lerp(This().CELL_3D_Z[lutPos0], This().CELL_3D_Z[lutPos1], xs);

	const v_flt lx0y = FNoiseMath::Lerp(lx0x, lx1x, ys);
	const v_flt ly0y = FNoiseMath::Lerp(ly0x, ly1x, ys);
	const v_flt lz0y = FNoiseMath::Lerp(lz0x, lz1x, ys);

	lutPos0 = This().Index3D_256(offset, x0, y0, z1);
	lutPos1 = This().Index3D_256(offset, x1, y0, z1);

	lx0x = FNoiseMath::Lerp(This().CELL_3D_X[lutPos0], This().CELL_3D_X[lutPos1], xs);
	ly0x = FNoiseMath::Lerp(This().CELL_3D_Y[lutPos0], This().CELL_3D_Y[lutPos1], xs);
	lz0x = FNoiseMath::Lerp(This().CELL_3D_Z[lutPos0], This().CELL_3D_Z[lutPos1], xs);

	lutPos0 = This().Index3D_256(offset, x0, y1, z1);
	lutPos1 = This().Index3D_256(offset, x1, y1, z1);

	lx1x = FNoiseMath::Lerp(This().CELL_3D_X[lutPos0], This().CELL_3D_X[lutPos1], xs);
	ly1x = FNoiseMath::Lerp(This().CELL_3D_Y[lutPos0], This().CELL_3D_Y[lutPos1], xs);
	lz1x = FNoiseMath::Lerp(This().CELL_3D_Z[lutPos0], This().CELL_3D_Z[lutPos1], xs);

	x += FNoiseMath::Lerp(lx0y, FNoiseMath::Lerp(lx0x, lx1x, ys), zs) * warpAmp;
	y += FNoiseMath::Lerp(ly0y, FNoiseMath::Lerp(ly0x, ly1x, ys), zs) * warpAmp;
	z += FNoiseMath::Lerp(lz0y, FNoiseMath::Lerp(lz0x, lz1x, ys), zs) * warpAmp;
}