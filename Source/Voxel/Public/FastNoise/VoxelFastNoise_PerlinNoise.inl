// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_PerlinNoise.h"
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
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_PerlinNoise<T>::SinglePerlin_2D(uint8 offset, v_flt x, v_flt y) const
{
	const int32 x0 = FNoiseMath::FastFloor(x);
	const int32 y0 = FNoiseMath::FastFloor(y);
	
	const int32 x1 = x0 + 1;
	const int32 y1 = y0 + 1;

	const v_flt fx = x - x0;
	const v_flt fy = y - y0;

	v_flt xs, ys;
	This().Interpolate_2D(fx, fy, xs, ys);

	const v_flt xd0 = fx;
	const v_flt yd0 = fy;
	
	const v_flt xd1 = xd0 - 1;
	const v_flt yd1 = yd0 - 1;

	const v_flt xf0 = FNoiseMath::Lerp(This().GradCoord2D(offset, x0, y0, xd0, yd0), This().GradCoord2D(offset, x1, y0, xd1, yd0), xs);
	const v_flt xf1 = FNoiseMath::Lerp(This().GradCoord2D(offset, x0, y1, xd0, yd1), This().GradCoord2D(offset, x1, y1, xd1, yd1), xs);

	return FNoiseMath::Lerp(xf0, xf1, ys);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_PerlinNoise<T>::SinglePerlin_2D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt& outDx, v_flt& outDy) const
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

	const v_flt xd0 = fx;
	const v_flt yd0 = fy;
	const v_flt xd1 = xd0 - 1;
	const v_flt yd1 = yd0 - 1;
	
    v_flt gax, gay;
    v_flt gbx, gby;
    v_flt gcx, gcy;
    v_flt gdx, gdy;
    
    const v_flt va = This().GradCoord2D(offset, x0, y0, xd0, yd0, gax, gay);
    const v_flt vb = This().GradCoord2D(offset, x1, y0, xd1, yd0, gbx, gby);
    const v_flt vc = This().GradCoord2D(offset, x0, y1, xd0, yd1, gcx, gcy);
    const v_flt vd = This().GradCoord2D(offset, x1, y1, xd1, yd1, gdx, gdy);

	outDx = gax + xs * (gbx - gax) + ys * (gcx - gax) + xs * ys * (gax - gbx - gcx + gdx) + dx * (ys * (va - vb - vc + vd) + vb - va);
	outDy = gay + xs * (gby - gay) + ys * (gcy - gay) + xs * ys * (gay - gby - gcy + gdy) + dy * (xs * (va - vb - vc + vd) + vc - va);

	return va + xs * (vb - va) + ys * (vc - va) + xs * ys * (va - vb - vc + vd);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_PerlinNoise<T>::SinglePerlin_3D(uint8 offset, v_flt x, v_flt y, v_flt z) const
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

	const v_flt xd0 = fx;
	const v_flt yd0 = fy;
	const v_flt zd0 = fz;
	const v_flt xd1 = xd0 - 1;
	const v_flt yd1 = yd0 - 1;
	const v_flt zd1 = zd0 - 1;

	const v_flt xf00 = FNoiseMath::Lerp(This().GradCoord3D(offset, x0, y0, z0, xd0, yd0, zd0), This().GradCoord3D(offset, x1, y0, z0, xd1, yd0, zd0), xs);
	const v_flt xf10 = FNoiseMath::Lerp(This().GradCoord3D(offset, x0, y1, z0, xd0, yd1, zd0), This().GradCoord3D(offset, x1, y1, z0, xd1, yd1, zd0), xs);
	const v_flt xf01 = FNoiseMath::Lerp(This().GradCoord3D(offset, x0, y0, z1, xd0, yd0, zd1), This().GradCoord3D(offset, x1, y0, z1, xd1, yd0, zd1), xs);
	const v_flt xf11 = FNoiseMath::Lerp(This().GradCoord3D(offset, x0, y1, z1, xd0, yd1, zd1), This().GradCoord3D(offset, x1, y1, z1, xd1, yd1, zd1), xs);

	const v_flt yf0 = FNoiseMath::Lerp(xf00, xf10, ys);
	const v_flt yf1 = FNoiseMath::Lerp(xf01, xf11, ys);

	return FNoiseMath::Lerp(yf0, yf1, zs);
}

template<typename T>
FN_FORCEINLINE_SINGLE v_flt TVoxelFastNoise_PerlinNoise<T>::SinglePerlin_3D_Deriv(uint8 offset, v_flt x, v_flt y, v_flt z, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
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

	const v_flt xd0 = fx;
	const v_flt yd0 = fy;
	const v_flt zd0 = fz;
	const v_flt xd1 = xd0 - 1;
	const v_flt yd1 = yd0 - 1;
	const v_flt zd1 = zd0 - 1;
	
    v_flt gax, gay, gaz;
    v_flt gbx, gby, gbz;
    v_flt gcx, gcy, gcz;
    v_flt gdx, gdy, gdz;
    v_flt gex, gey, gez;
    v_flt gfx, gfy, gfz;
    v_flt ggx, ggy, ggz;
    v_flt ghx, ghy, ghz;
    
    const v_flt va = This().GradCoord3D(offset, x0, y0, z0, xd0, yd0, zd0, gax, gay, gaz);
    const v_flt vb = This().GradCoord3D(offset, x1, y0, z0, xd1, yd0, zd0, gbx, gby, gbz);
    const v_flt vc = This().GradCoord3D(offset, x0, y1, z0, xd0, yd1, zd0, gcx, gcy, gcz);
    const v_flt vd = This().GradCoord3D(offset, x1, y1, z0, xd1, yd1, zd0, gdx, gdy, gdz);
    const v_flt ve = This().GradCoord3D(offset, x0, y0, z1, xd0, yd0, zd1, gex, gey, gez);
    const v_flt vf = This().GradCoord3D(offset, x1, y0, z1, xd1, yd0, zd1, gfx, gfy, gfz);
    const v_flt vg = This().GradCoord3D(offset, x0, y1, z1, xd0, yd1, zd1, ggx, ggy, ggz);
	const v_flt vh = This().GradCoord3D(offset, x1, y1, z1, xd1, yd1, zd1, ghx, ghy, ghz);

	outDx =
		gax +
		xs * (gbx - gax) +
		ys * (gcx - gax) +
		zs * (gex - gax) +
		xs * ys * (gax - gbx - gcx + gdx) +
		ys * zs * (gax - gcx - gex + ggx) +
		zs * xs * (gax - gbx - gex + gfx) +
		xs * ys * zs * (-gax + gbx + gcx - gdx + gex - gfx - ggx + ghx) +

		dx * (
			vb - va +
			ys * (va - vb - vc + vd) +
			zs * (va - vb - ve + vf) +
			ys * zs * (-va + vb + vc - vd + ve - vf - vg + vh));

	outDy =
		gay +
		xs * (gby - gay) +
		ys * (gcy - gay) +
		zs * (gey - gay) +
		xs * ys * (gay - gby - gcy + gdy) +
		ys * zs * (gay - gcy - gey + ggy) +
		zs * xs * (gay - gby - gey + gfy) +
		xs * ys * zs * (-gay + gby + gcy - gdy + gey - gfy - ggy + ghy) +

		dy * (
			vc - va +
			zs * (va - vc - ve + vg) +
			xs * (va - vb - vc + vd) +
			zs * xs * (-va + vb + vc - vd + ve - vf - vg + vh));

	outDz =
		gaz +
		xs * (gbz - gaz) +
		ys * (gcz - gaz) +
		zs * (gez - gaz) +
		xs * ys * (gaz - gbz - gcz + gdz) +
		ys * zs * (gaz - gcz - gez + ggz) +
		zs * xs * (gaz - gbz - gez + gfz) +
		xs * ys * zs * (-gaz + gbz + gcz - gdz + gez - gfz - ggz + ghz) +

		dz * (
			ve - va +
			xs * (va - vb - ve + vf) +
			ys * (va - vc - ve + vg) +
			xs * ys * (-va + vb + vc - vd + ve - vf - vg + vh));

	return 
		va + 
		xs * (vb - va) + 
		ys * (vc - va) + 
		zs * (ve - va) + 
		xs * ys * (va - vb - vc + vd) + 
		ys * zs * (va - vc - ve + vg) + 
		zs * xs * (va - vb - ve + vf) + 
		xs * ys * zs * (-va + vb + vc - vd + ve - vf - vg + vh);
}