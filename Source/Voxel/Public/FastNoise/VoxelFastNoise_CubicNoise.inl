// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_CubicNoise.h"
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
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CubicNoise<T>::SingleCubic_2D(uint8 offset, v_flt x, v_flt y) const
{
	const int32 x1 = FNoiseMath::FastFloor(x);
	const int32 y1 = FNoiseMath::FastFloor(y);

	const int32 x0 = x1 - 1;
	const int32 y0 = y1 - 1;
	const int32 x2 = x1 + 1;
	const int32 y2 = y1 + 1;
	const int32 x3 = x1 + 2;
	const int32 y3 = y1 + 2;

	const v_flt xs = x - v_flt(x1);
	const v_flt ys = y - v_flt(y1);

	return FNoiseMath::CubicLerp(
		FNoiseMath::CubicLerp(This().ValCoord2DFast(offset, x0, y0), This().ValCoord2DFast(offset, x1, y0), This().ValCoord2DFast(offset, x2, y0), This().ValCoord2DFast(offset, x3, y0), xs),
		FNoiseMath::CubicLerp(This().ValCoord2DFast(offset, x0, y1), This().ValCoord2DFast(offset, x1, y1), This().ValCoord2DFast(offset, x2, y1), This().ValCoord2DFast(offset, x3, y1), xs),
		FNoiseMath::CubicLerp(This().ValCoord2DFast(offset, x0, y2), This().ValCoord2DFast(offset, x1, y2), This().ValCoord2DFast(offset, x2, y2), This().ValCoord2DFast(offset, x3, y2), xs),
		FNoiseMath::CubicLerp(This().ValCoord2DFast(offset, x0, y3), This().ValCoord2DFast(offset, x1, y3), This().ValCoord2DFast(offset, x2, y3), This().ValCoord2DFast(offset, x3, y3), xs),
		ys) * CUBIC_2D_BOUNDING;
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_CubicNoise<T>::SingleCubic_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const
{
	const int32 x1 = FNoiseMath::FastFloor(x);
	const int32 y1 = FNoiseMath::FastFloor(y);
	const int32 z1 = FNoiseMath::FastFloor(z);

	const int32 x0 = x1 - 1;
	const int32 y0 = y1 - 1;
	const int32 z0 = z1 - 1;
	const int32 x2 = x1 + 1;
	const int32 y2 = y1 + 1;
	const int32 z2 = z1 + 1;
	const int32 x3 = x1 + 2;
	const int32 y3 = y1 + 2;
	const int32 z3 = z1 + 2;

	const v_flt xs = x - v_flt(x1);
	const v_flt ys = y - v_flt(y1);
	const v_flt zs = z - v_flt(z1);

	return FNoiseMath::CubicLerp(
		FNoiseMath::CubicLerp(
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y0, z0), This().ValCoord3DFast(offset, x1, y0, z0), This().ValCoord3DFast(offset, x2, y0, z0), This().ValCoord3DFast(offset, x3, y0, z0), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y1, z0), This().ValCoord3DFast(offset, x1, y1, z0), This().ValCoord3DFast(offset, x2, y1, z0), This().ValCoord3DFast(offset, x3, y1, z0), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y2, z0), This().ValCoord3DFast(offset, x1, y2, z0), This().ValCoord3DFast(offset, x2, y2, z0), This().ValCoord3DFast(offset, x3, y2, z0), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y3, z0), This().ValCoord3DFast(offset, x1, y3, z0), This().ValCoord3DFast(offset, x2, y3, z0), This().ValCoord3DFast(offset, x3, y3, z0), xs),
			ys),
		FNoiseMath::CubicLerp(
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y0, z1), This().ValCoord3DFast(offset, x1, y0, z1), This().ValCoord3DFast(offset, x2, y0, z1), This().ValCoord3DFast(offset, x3, y0, z1), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y1, z1), This().ValCoord3DFast(offset, x1, y1, z1), This().ValCoord3DFast(offset, x2, y1, z1), This().ValCoord3DFast(offset, x3, y1, z1), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y2, z1), This().ValCoord3DFast(offset, x1, y2, z1), This().ValCoord3DFast(offset, x2, y2, z1), This().ValCoord3DFast(offset, x3, y2, z1), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y3, z1), This().ValCoord3DFast(offset, x1, y3, z1), This().ValCoord3DFast(offset, x2, y3, z1), This().ValCoord3DFast(offset, x3, y3, z1), xs),
			ys),
		FNoiseMath::CubicLerp(
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y0, z2), This().ValCoord3DFast(offset, x1, y0, z2), This().ValCoord3DFast(offset, x2, y0, z2), This().ValCoord3DFast(offset, x3, y0, z2), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y1, z2), This().ValCoord3DFast(offset, x1, y1, z2), This().ValCoord3DFast(offset, x2, y1, z2), This().ValCoord3DFast(offset, x3, y1, z2), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y2, z2), This().ValCoord3DFast(offset, x1, y2, z2), This().ValCoord3DFast(offset, x2, y2, z2), This().ValCoord3DFast(offset, x3, y2, z2), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y3, z2), This().ValCoord3DFast(offset, x1, y3, z2), This().ValCoord3DFast(offset, x2, y3, z2), This().ValCoord3DFast(offset, x3, y3, z2), xs),
			ys),
		FNoiseMath::CubicLerp(
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y0, z3), This().ValCoord3DFast(offset, x1, y0, z3), This().ValCoord3DFast(offset, x2, y0, z3), This().ValCoord3DFast(offset, x3, y0, z3), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y1, z3), This().ValCoord3DFast(offset, x1, y1, z3), This().ValCoord3DFast(offset, x2, y1, z3), This().ValCoord3DFast(offset, x3, y1, z3), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y2, z3), This().ValCoord3DFast(offset, x1, y2, z3), This().ValCoord3DFast(offset, x2, y2, z3), This().ValCoord3DFast(offset, x3, y2, z3), xs),
			FNoiseMath::CubicLerp(This().ValCoord3DFast(offset, x0, y3, z3), This().ValCoord3DFast(offset, x1, y3, z3), This().ValCoord3DFast(offset, x2, y3, z3), This().ValCoord3DFast(offset, x3, y3, z3), xs),
			ys),
		zs) * CUBIC_3D_BOUNDING;
}