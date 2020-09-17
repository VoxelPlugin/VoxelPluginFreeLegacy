// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_ValueNoise.h"
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
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_ValueNoise<T>::SingleValue_2D(uint8 offset, v_flt x, v_flt y) const
{
	const int32 x0 = FNoiseMath::FastFloor(x);
	const int32 y0 = FNoiseMath::FastFloor(y);
	
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;

	const v_flt fx = x - x0;
	const v_flt fy = y - y0;

	v_flt xs, ys;
	This().Interpolate_2D(fx, fy, xs, ys);

	const v_flt xf0 = FNoiseMath::Lerp(This().ValCoord2DFast(offset, x0, y0), This().ValCoord2DFast(offset, x1, y0), xs);
	const v_flt xf1 = FNoiseMath::Lerp(This().ValCoord2DFast(offset, x0, y1), This().ValCoord2DFast(offset, x1, y1), xs);

	return FNoiseMath::Lerp(xf0, xf1, ys);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_ValueNoise<T>::SingleValue_2D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt& outDx, v_flt& outDy) const
{
	const int32 x0 = FNoiseMath::FastFloor(x);
	const int32 y0 = FNoiseMath::FastFloor(y);
	
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;
	
	const v_flt fx = x - x0;
	const v_flt fy = y - y0;

	v_flt xs, ys;
	v_flt dx, dy;
	This().Interpolate_2D_Deriv(fx, fy, xs, ys, dx, dy);

	const v_flt a = This().ValCoord2DFast(offset, x0, y0);
	const v_flt b = This().ValCoord2DFast(offset, x1, y0);
	const v_flt c = This().ValCoord2DFast(offset, x0, y1);
	const v_flt d = This().ValCoord2DFast(offset, x1, y1);
	
	outDx = dx * (ys * (a - b - c + d) + b - a);
	outDy = dy * (xs * (a - b - c + d) + c - a);

	return a + (b - a) * xs + (c - a) * ys + (a - b - c + d) * xs * ys;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_ValueNoise<T>::SingleValue_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const
{
	const int32 x0 = FNoiseMath::FastFloor(x);
	const int32 y0 = FNoiseMath::FastFloor(y);
	const int32 z0 = FNoiseMath::FastFloor(z);
	
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;
	const int32 z1 = z0 + 1;
	
	const v_flt fx = x - x0;
	const v_flt fy = y - y0;
	const v_flt fz = z - z0;

	v_flt xs, ys, zs;
	This().Interpolate_3D(fx, fy, fz, xs, ys, zs);

	const v_flt xf00 = FNoiseMath::Lerp(This().ValCoord3DFast(offset, x0, y0, z0), This().ValCoord3DFast(offset, x1, y0, z0), xs);
	const v_flt xf10 = FNoiseMath::Lerp(This().ValCoord3DFast(offset, x0, y1, z0), This().ValCoord3DFast(offset, x1, y1, z0), xs);
	const v_flt xf01 = FNoiseMath::Lerp(This().ValCoord3DFast(offset, x0, y0, z1), This().ValCoord3DFast(offset, x1, y0, z1), xs);
	const v_flt xf11 = FNoiseMath::Lerp(This().ValCoord3DFast(offset, x0, y1, z1), This().ValCoord3DFast(offset, x1, y1, z1), xs);

	const v_flt yf0 = FNoiseMath::Lerp(xf00, xf10, ys);
	const v_flt yf1 = FNoiseMath::Lerp(xf01, xf11, ys);

	return FNoiseMath::Lerp(yf0, yf1, zs);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_ValueNoise<T>::SingleValue_3D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt z, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
	const int32 x0 = FNoiseMath::FastFloor(x);
	const int32 y0 = FNoiseMath::FastFloor(y);
	const int32 z0 = FNoiseMath::FastFloor(z);
	
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;
	const int32 z1 = z0 + 1;
	
	const v_flt fx = x - x0;
	const v_flt fy = y - y0;
	const v_flt fz = z - z0;

	v_flt xs, ys, zs;
	v_flt dx, dy, dz;
	This().Interpolate_3D_Deriv(fx, fy, fz, xs, ys, zs, dx, dy, dz);

	const v_flt a = This().ValCoord3DFast(offset, x0, y0, z0);
	const v_flt b = This().ValCoord3DFast(offset, x1, y0, z0);
	const v_flt c = This().ValCoord3DFast(offset, x0, y1, z0);
	const v_flt d = This().ValCoord3DFast(offset, x1, y1, z0);
	const v_flt e = This().ValCoord3DFast(offset, x0, y0, z1);
	const v_flt f = This().ValCoord3DFast(offset, x1, y0, z1);
	const v_flt g = This().ValCoord3DFast(offset, x0, y1, z1);
	const v_flt h = This().ValCoord3DFast(offset, x1, y1, z1);

	const v_flt k0 = a;
	const v_flt k1 = b - a;
	const v_flt k2 = c - a;
	const v_flt k3 = e - a;
	const v_flt k4 = a - b - c + d;
	const v_flt k5 = a - c - e + g;
	const v_flt k6 = a - b - e + f;
	const v_flt k7 = -a + b + c - d + e - f - g + h;

	outDx = dx * (k1 + k4 * ys + k6 * zs + k7 * ys * zs);
	outDy = dy * (k2 + k5 * zs + k4 * xs + k7 * zs * xs);
	outDz = dz * (k3 + k6 * xs + k5 * ys + k7 * xs * ys);

	return k0 + k1 * xs + k2 * ys + k3 * zs + k4 * xs * ys + k5 * ys * zs + k6 * zs * xs + k7 * xs * ys * zs;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
VectorRegister TVoxelFastNoise_ValueNoise<T>::SingleValue_2D(VectorRegisterInt offset, VectorRegister x, VectorRegister y) const
{
	const VectorRegister x0f = VectorFloor(x);
	const VectorRegister y0f = VectorFloor(y);

	const VectorRegisterInt x0 = VectorFloatToInt(x0f);
	const VectorRegisterInt y0 = VectorFloatToInt(y0f);

	const VectorRegisterInt x1 = VectorIntAdd(x0, GlobalVectorConstants::IntOne);
	const VectorRegisterInt y1 = VectorIntAdd(y0, GlobalVectorConstants::IntOne);

	const VectorRegister fx = VectorSubtract(x, x0f);
	const VectorRegister fy = VectorSubtract(y, y0f);

	VectorRegister xs, ys;
	This().Interpolate_2D(fx, fy, xs, ys);

	const VectorRegister xf0 = FNoiseMath::Lerp(This().ValCoord2DFast(offset, x0, y0), This().ValCoord2DFast(offset, x1, y0), xs);
	const VectorRegister xf1 = FNoiseMath::Lerp(This().ValCoord2DFast(offset, x0, y1), This().ValCoord2DFast(offset, x1, y1), xs);

	return FNoiseMath::Lerp(xf0, xf1, ys);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_ValueNoise<T>::IQNoise_2D(v_flt x, v_flt y, v_flt frequency, int32 octaves) const
{
	v_flt dx, dy;
	return IQNoise_2D_Deriv(x, y, frequency, octaves, dx, dy);
}

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_ValueNoise<T>::IQNoise_2D_Deriv(v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const
{
	x *= frequency;
	y *= frequency;

	v_flt sum = SingleValue_2D_Deriv(This().Perm[0], x, y, outDx, outDy);
	v_flt amp = 1;
	int32 i = 0;

	v_flt localDx = outDx;
	v_flt localDy = outDy;

	while (++i < octaves)
	{
		x *= This().Lacunarity;
		y *= This().Lacunarity;
		const FVector2D P = This().Matrix2.TransformPoint({ float(x), float(y) });
		x = P.X;
		y = P.Y;

		amp *= This().Gain;

		v_flt dx, dy;
		const v_flt value = SingleValue_2D_Deriv(This().Perm[i], x, y, dx, dy);

		localDx += dx;
		localDy += dy;

		const v_flt multiplier = amp / (1 + localDx * localDx + localDy * localDy);;
		sum += value * multiplier;

		// Not exact, but still gives good results
		outDx += dx * multiplier;
		outDy += dy * multiplier;
	}

	outDx *= This().FractalBounding;
	outDy *= This().FractalBounding;
	return sum * This().FractalBounding;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_ValueNoise<T>::IQNoise_3D(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves) const
{
	v_flt dx, dy, dz;
	return IQNoise_3D_Deriv(x, y, z, frequency, octaves, dx, dy, dz);
}

template<typename T>
FN_FORCEINLINE v_flt TVoxelFastNoise_ValueNoise<T>::IQNoise_3D_Deriv(v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
	x *= frequency;
	y *= frequency;
	z *= frequency;

	v_flt sum = SingleValue_3D_Deriv(This().Perm[0], x, y, z, outDx, outDy, outDz);
	v_flt amp = 1;
	int32 i = 0;

	v_flt localDx = outDx;
	v_flt localDy = outDy;
	v_flt localDz = outDz;

	while (++i < octaves)
	{
		x *= This().Lacunarity;
		y *= This().Lacunarity;
		z *= This().Lacunarity;

		const FVector4 P = This().Matrix3.TransformPosition({ float(x), float(y), float(z) });
		x = P.X;
		y = P.Y;
		z = P.Z;

		amp *= This().Gain;

		v_flt dx, dy, dz;
		const v_flt value = SingleValue_3D_Deriv(This().Perm[i], x, y, z, dx, dy, dz);

		localDx += dx;
		localDy += dy;
		localDz += dz;

		const v_flt multiplier = amp / (1 + localDx * localDx + localDy * localDy + localDz * localDz);;
		sum += value * multiplier;

		// Not exact, but still gives good results
		outDx += dx * multiplier;
		outDy += dy * multiplier;
		outDy += dz * multiplier;
	}

	outDx *= This().FractalBounding;
	outDy *= This().FractalBounding;
	outDz *= This().FractalBounding;
	return sum * This().FractalBounding;
}