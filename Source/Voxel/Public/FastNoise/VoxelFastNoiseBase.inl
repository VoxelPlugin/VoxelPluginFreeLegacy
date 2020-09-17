// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoiseBase.h"
#include "FastNoise/VoxelFastNoiseLUT.inl"

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

#define VOXEL_FRACTAL_TYPE_SWITCH(Macro) \
	switch (This().FractalType) \
	{ \
	default: ensureVoxelSlow(false); \
	case EVoxelNoiseFractalType::FBM: \
		return Macro(FBM); \
	case EVoxelNoiseFractalType::Billow: \
		return Macro(Billow); \
	case EVoxelNoiseFractalType::RigidMulti: \
		return Macro(RigidMulti); \
	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH void FVoxelFastNoiseBase::CalculateFractalBounding(int32 Octaves)
{
	float amp = Gain;
	float ampFractal = 1.0f;
	for (int32 Index = 1; Index < Octaves; Index++)
	{
		ampFractal += amp;
		amp *= Gain;
	}
	FractalBounding = 1.0f / ampFractal;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_MATH void FVoxelFastNoiseBase::Interpolate_2D(T fx, T fy, T& xs, T& ys) const
{
	switch (Interpolation)
	{
	default: ensureVoxelSlow(false);
	case EVoxelNoiseInterpolation::Linear:
		xs = fx;
		ys = fy;
		break;
	case EVoxelNoiseInterpolation::Hermite:
		xs = FNoiseMath::InterpHermiteFunc(fx);
		ys = FNoiseMath::InterpHermiteFunc(fy);
		break;
	case EVoxelNoiseInterpolation::Quintic:
		xs = FNoiseMath::InterpQuinticFunc(fx);
		ys = FNoiseMath::InterpQuinticFunc(fy);
		break;
	}
}

template<typename T>
FN_FORCEINLINE_MATH void FVoxelFastNoiseBase::Interpolate_2D_Deriv(T fx, T fy, T& xs, T& ys, T& dx, T& dy) const
{
	switch (Interpolation)
	{
	default: ensureVoxelSlow(false);
	case EVoxelNoiseInterpolation::Linear:
		xs = fx;
		ys = fy;
		dx = 1;
		dy = 1;
		break;
	case EVoxelNoiseInterpolation::Hermite:
		xs = FNoiseMath::InterpHermiteFunc(fx);
		ys = FNoiseMath::InterpHermiteFunc(fy);
		dx = FNoiseMath::InterpHermiteFuncDeriv(fx);
		dy = FNoiseMath::InterpHermiteFuncDeriv(fy);
		break;
	case EVoxelNoiseInterpolation::Quintic:
		xs = FNoiseMath::InterpQuinticFunc(fx);
		ys = FNoiseMath::InterpQuinticFunc(fy);
		dx = FNoiseMath::InterpQuinticFuncDeriv(fx);
		dy = FNoiseMath::InterpQuinticFuncDeriv(fy);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_MATH void FVoxelFastNoiseBase::Interpolate_3D(T fx, T fy, T fz, T& xs, T& ys, T& zs) const
{
	switch (Interpolation)
	{
	default: ensureVoxelSlow(false);
	case EVoxelNoiseInterpolation::Linear:
		xs = fx;
		ys = fy;
		zs = fz;
		break;
	case EVoxelNoiseInterpolation::Hermite:
		xs = FNoiseMath::InterpHermiteFunc(fx);
		ys = FNoiseMath::InterpHermiteFunc(fy);
		zs = FNoiseMath::InterpHermiteFunc(fz);
		break;
	case EVoxelNoiseInterpolation::Quintic:
		xs = FNoiseMath::InterpQuinticFunc(fx);
		ys = FNoiseMath::InterpQuinticFunc(fy);
		zs = FNoiseMath::InterpQuinticFunc(fz);
		break;
	}
}

template<typename T>
FN_FORCEINLINE_MATH void FVoxelFastNoiseBase::Interpolate_3D_Deriv(T fx, T fy, T fz, T& xs, T& ys, T& zs, T& dx, T& dy, T& dz) const
{
	switch (Interpolation)
	{
	default: ensureVoxelSlow(false);
	case EVoxelNoiseInterpolation::Linear:
		xs = fx;
		ys = fy;
		zs = fz;
		dx = 1;
		dy = 1;
		dz = 1;
		break;
	case EVoxelNoiseInterpolation::Hermite:
		xs = FNoiseMath::InterpHermiteFunc(fx);
		ys = FNoiseMath::InterpHermiteFunc(fy);
		zs = FNoiseMath::InterpHermiteFunc(fz);
		dx = FNoiseMath::InterpHermiteFuncDeriv(fx);
		dy = FNoiseMath::InterpHermiteFuncDeriv(fy);
		dz = FNoiseMath::InterpHermiteFuncDeriv(fz);
		break;
	case EVoxelNoiseInterpolation::Quintic:
		xs = FNoiseMath::InterpQuinticFunc(fx);
		ys = FNoiseMath::InterpQuinticFunc(fy);
		zs = FNoiseMath::InterpQuinticFunc(fz);
		dx = FNoiseMath::InterpQuinticFuncDeriv(fx);
		dy = FNoiseMath::InterpQuinticFuncDeriv(fy);
		dz = FNoiseMath::InterpQuinticFuncDeriv(fz);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::Fractal_2D(T GetNoise, v_flt x, v_flt y, v_flt frequency, int32 octaves) const
{
#define Macro(Type) Fractal##Type##_2D(GetNoise, x * frequency, y * frequency, octaves)
	VOXEL_FRACTAL_TYPE_SWITCH(Macro)
#undef Macro
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::Fractal_2D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy) const
{
#define Macro(Type) Fractal##Type##_2D_Deriv(GetNoise, x * frequency, y * frequency, octaves, outDx, outDy)
	VOXEL_FRACTAL_TYPE_SWITCH(Macro)
#undef Macro
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalFBM_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const
{
	v_flt sum = GetNoise(Perm[0], x, y);
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;

		amp *= Gain;
		
		sum += GetNoise(Perm[i], x, y) * amp;
	}

	return sum * FractalBounding;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalFBM_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const
{
	v_flt sum = GetNoise(Perm[0], x, y, outDx, outDy);
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;

		amp *= Gain;

		v_flt dx, dy;
		sum += GetNoise(Perm[i], x, y, dx, dy) * amp;
		
		outDx += amp * dx;
		outDy += amp * dy;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	return sum * FractalBounding;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalBillow_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const
{
	v_flt sum = FNoiseMath::FastAbs(GetNoise(Perm[0], x, y)) * 2 - 1;
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		amp *= Gain;
		sum += (FNoiseMath::FastAbs(GetNoise(Perm[i], x, y)) * 2 - 1) * amp;
	}

	return sum * FractalBounding;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalBillow_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const
{
	v_flt dx, dy;
	v_flt value = GetNoise(Perm[0], x, y, dx, dy);

	v_flt sum = FNoiseMath::FastAbs(value) * 2 - 1;
	outDx = FNoiseMath::FastAbsDeriv(value, dx) * 2;
	outDy = FNoiseMath::FastAbsDeriv(value, dy) * 2;

	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		amp *= Gain;

		value = GetNoise(Perm[i], x, y, dx, dy);

		sum += (FNoiseMath::FastAbs(value) * 2 - 1) * amp;
		outDx += FNoiseMath::FastAbsDeriv(value, dx) * 2 * amp;
		outDy += FNoiseMath::FastAbsDeriv(value, dy) * 2 * amp;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	return sum * FractalBounding;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalRigidMulti_2D(T GetNoise, v_flt x, v_flt y, int32 octaves) const
{
	v_flt sum = 1 - FNoiseMath::FastAbs(GetNoise(Perm[0], x, y));
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;

		amp *= Gain;
		sum -= (1 - FNoiseMath::FastAbs(GetNoise(Perm[i], x, y))) * amp;
	}

	return sum;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalRigidMulti_2D_Deriv(T GetNoise, v_flt x, v_flt y, int32 octaves, v_flt& outDx, v_flt& outDy) const
{
	v_flt dx, dy;
	v_flt value = GetNoise(Perm[0], x, y, dx, dy);

	v_flt sum = 1 - FNoiseMath::FastAbs(value);
	outDx = -FNoiseMath::FastAbsDeriv(value, dx);
	outDy = -FNoiseMath::FastAbsDeriv(value, dy);

	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		amp *= Gain;

		value = GetNoise(Perm[i], x, y, dx, dy);

		sum -= (1 - FNoiseMath::FastAbs(value)) * amp;
		outDx -= -FNoiseMath::FastAbsDeriv(value, dx) * amp;
		outDy -= -FNoiseMath::FastAbsDeriv(value, dy) * amp;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	return sum;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::Fractal_3D(T GetNoise, v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves) const
{
#define Macro(Type) Fractal##Type##_3D(GetNoise, x * frequency, y * frequency, z * frequency, octaves)
	VOXEL_FRACTAL_TYPE_SWITCH(Macro)
#undef Macro
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::Fractal_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, v_flt frequency, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
#define Macro(Type) Fractal##Type##_3D_Deriv(GetNoise, x * frequency, y * frequency, z * frequency, octaves, outDx, outDy, outDz)
	VOXEL_FRACTAL_TYPE_SWITCH(Macro)
#undef Macro
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalFBM_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const
{
	v_flt sum = GetNoise(Perm[0], x, y, z);
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;

		amp *= Gain;
		
		sum += GetNoise(Perm[i], x, y, z) * amp;
	}

	return sum * FractalBounding;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalFBM_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
	v_flt sum = GetNoise(Perm[0], x, y, z, outDx, outDy, outDz);
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;

		amp *= Gain;

		v_flt dx, dy, dz;
		sum += GetNoise(Perm[i], x, y, z, dx, dy, dz) * amp;
		
		outDx += amp * dx;
		outDy += amp * dy;
		outDz += amp * dz;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	outDz *= FractalBounding;
	return sum * FractalBounding;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalBillow_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const
{
	v_flt sum = FNoiseMath::FastAbs(GetNoise(Perm[0], x, y, z)) * 2 - 1;
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		amp *= Gain;
		sum += (FNoiseMath::FastAbs(GetNoise(Perm[i], x, y, z)) * 2 - 1) * amp;
	}

	return sum * FractalBounding;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalBillow_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
	v_flt dx, dy, dz;
	v_flt value = GetNoise(Perm[0], x, y, z, dx, dy, dz);

	v_flt sum = FNoiseMath::FastAbs(value) * 2 - 1;
	outDx = FNoiseMath::FastAbsDeriv(value, dx) * 2;
	outDy = FNoiseMath::FastAbsDeriv(value, dy) * 2;
	outDz = FNoiseMath::FastAbsDeriv(value, dz) * 2;

	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		amp *= Gain;

		value = GetNoise(Perm[i], x, y, z, dx, dy, dz);

		sum += (FNoiseMath::FastAbs(value) * 2 - 1) * amp;
		outDx += FNoiseMath::FastAbsDeriv(value, dx) * 2 * amp;
		outDy += FNoiseMath::FastAbsDeriv(value, dy) * 2 * amp;
		outDz += FNoiseMath::FastAbsDeriv(value, dz) * 2 * amp;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	outDz *= FractalBounding;
	return sum * FractalBounding;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalRigidMulti_3D(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves) const
{
	v_flt sum = 1 - FNoiseMath::FastAbs(GetNoise(Perm[0], x, y, z));
	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;

		amp *= Gain;
		sum -= (1 - FNoiseMath::FastAbs(GetNoise(Perm[i], x, y, z))) * amp;
	}

	return sum;
}

template<typename T>
FN_FORCEINLINE v_flt FVoxelFastNoiseBase::FractalRigidMulti_3D_Deriv(T GetNoise, v_flt x, v_flt y, v_flt z, int32 octaves, v_flt& outDx, v_flt& outDy, v_flt& outDz) const
{
	v_flt dx, dy, dz;
	v_flt value = GetNoise(Perm[0], x, y, z, dx, dy, dz);

	v_flt sum = 1 - FNoiseMath::FastAbs(value);
	outDx = -FNoiseMath::FastAbsDeriv(value, dx);
	outDy = -FNoiseMath::FastAbsDeriv(value, dy);
	outDz = -FNoiseMath::FastAbsDeriv(value, dz);

	v_flt amp = 1;
	int32 i = 0;

	while (++i < octaves)
	{
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		amp *= Gain;

		value = GetNoise(Perm[i], x, y, z, dx, dy, dz);

		sum -= (1 - FNoiseMath::FastAbs(value)) * amp;
		outDx -= -FNoiseMath::FastAbsDeriv(value, dx) * amp;
		outDy -= -FNoiseMath::FastAbsDeriv(value, dy) * amp;
		outDz -= -FNoiseMath::FastAbsDeriv(value, dz) * amp;
	}

	outDx *= FractalBounding;
	outDy *= FractalBounding;
	outDz *= FractalBounding;
	return sum;
}