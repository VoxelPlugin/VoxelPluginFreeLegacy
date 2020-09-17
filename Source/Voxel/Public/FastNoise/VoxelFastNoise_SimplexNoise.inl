// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_SimplexNoise.h"
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
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_SimplexNoise<T>::SingleSimplex_2D(uint8 offset, v_flt x, v_flt y) const
{
	v_flt t = (x + y) * F2;
	const int32 i = FNoiseMath::FastFloor(x + t);
	const int32 j = FNoiseMath::FastFloor(y + t);

	t = (i + j) * G2;
	const v_flt X0 = i - t;
	const v_flt Y0 = j - t;

	const v_flt x0 = x - X0;
	const v_flt y0 = y - Y0;

	int32 i1, j1;
	if (x0 > y0)
	{
		i1 = 1; j1 = 0;
	}
	else
	{
		i1 = 0; j1 = 1;
	}

	const v_flt x1 = x0 - v_flt(i1) + G2;
	const v_flt y1 = y0 - v_flt(j1) + G2;
	const v_flt x2 = x0 - 1 + 2 * G2;
	const v_flt y2 = y0 - 1 + 2 * G2;

	v_flt n0, n1, n2;

	t = v_flt(0.5) - x0 * x0 - y0 * y0;
	if (t < 0) 
	{
		n0 = 0;
	}
	else
	{
		t *= t;
		n0 = t * t * This().GradCoord2D(offset, i, j, x0, y0);
	}

	t = v_flt(0.5) - x1 * x1 - y1 * y1;
	if (t < 0)
	{
		n1 = 0;
	}
	else
	{
		t *= t;
		n1 = t * t * This().GradCoord2D(offset, i + i1, j + j1, x1, y1);
	}

	t = v_flt(0.5) - x2 * x2 - y2 * y2;
	if (t < 0)
	{
		n2 = 0;
	}
	else
	{
		t *= t;
		n2 = t * t * This().GradCoord2D(offset, i + 1, j + 1, x2, y2);
	}

	return 70 * (n0 + n1 + n2);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_SimplexNoise<T>::SingleSimplex_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const
{
	v_flt t = (x + y + z) * F3;
	const int32 i = FNoiseMath::FastFloor(x + t);
	const int32 j = FNoiseMath::FastFloor(y + t);
	const int32 k = FNoiseMath::FastFloor(z + t);

	t = (i + j + k) * G3;
	const v_flt X0 = i - t;
	const v_flt Y0 = j - t;
	const v_flt Z0 = k - t;

	const v_flt x0 = x - X0;
	const v_flt y0 = y - Y0;
	const v_flt z0 = z - Z0;

	int32 i1, j1, k1;
	int32 i2, j2, k2;

	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
		}
		else if (x0 >= z0)
		{
			i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
		}
		else // x0 < z0
		{
			i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
		}
	}
	else // x0 < y0
	{
		if (y0 < z0)
		{
			i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
		}
		else if (x0 < z0)
		{
			i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
		}
		else // x0 >= z0
		{
			i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
		}
	}

	const v_flt x1 = x0 - i1 + G3;
	const v_flt y1 = y0 - j1 + G3;
	const v_flt z1 = z0 - k1 + G3;
	
	const v_flt x2 = x0 - i2 + 2 * G3;
	const v_flt y2 = y0 - j2 + 2 * G3;
	const v_flt z2 = z0 - k2 + 2 * G3;
	
	const v_flt x3 = x0 - 1 + 3 * G3;
	const v_flt y3 = y0 - 1 + 3 * G3;
	const v_flt z3 = z0 - 1 + 3 * G3;

	v_flt n0, n1, n2, n3;

	t = v_flt(0.6) - x0 * x0 - y0 * y0 - z0 * z0;
	if (t < 0)
	{
		n0 = 0;
	}
	else
	{
		t *= t;
		n0 = t * t * This().GradCoord3D(offset, i, j, k, x0, y0, z0);
	}

	t = v_flt(0.6) - x1 * x1 - y1 * y1 - z1 * z1;
	if (t < 0)
	{
		n1 = 0;
	}
	else
	{
		t *= t;
		n1 = t * t * This().GradCoord3D(offset, i + i1, j + j1, k + k1, x1, y1, z1);
	}

	t = v_flt(0.6) - x2 * x2 - y2 * y2 - z2 * z2;
	if (t < 0)
	{
		n2 = 0;
	}
	else
	{
		t *= t;
		n2 = t * t * This().GradCoord3D(offset, i + i2, j + j2, k + k2, x2, y2, z2);
	}

	t = v_flt(0.6) - x3 * x3 - y3 * y3 - z3 * z3;
	if (t < 0)
	{
		n3 = 0;
	}
	else
	{
		t *= t;
		n3 = t * t * This().GradCoord3D(offset, i + 1, j + 1, k + 1, x3, y3, z3);
	}

	return 32 * (n0 + n1 + n2 + n3);
}