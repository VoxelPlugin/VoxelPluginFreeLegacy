// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoiseMath.h"
#include <cmath>

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

FN_FORCEINLINE_MATH int32 FVoxelFastNoiseMath::FastFloor(v_flt f)
{
	// Faster than FMath::FloorToInt
	return (f >= 0 ? (int32)f : (int32)f - 1);
}
FN_FORCEINLINE_MATH int32 FVoxelFastNoiseMath::FastRound(v_flt f)
{
	return (f >= 0) ? (int32)(f + v_flt(0.5)) : (int32)(f - v_flt(0.5));
}
FN_FORCEINLINE_MATH int32 FVoxelFastNoiseMath::FastAbs(int32 i)
{
	return abs(i);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::FastAbs(v_flt f)
{
	return fabs(f);
}
FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::FastAbsDeriv(v_flt x, v_flt dx)
{
	return dx * (x < 0 ? -1 : 1);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::Lerp(v_flt a, v_flt b, v_flt t)
{
	return a + t * (b - a);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::InterpHermiteFunc(v_flt t)
{
	return t*t*(3 - 2 * t);
}
FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::InterpHermiteFuncDeriv(v_flt t)
{
	return -6 * t * (t - 1);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::InterpQuinticFunc(v_flt t)
{
	return t*t*t*(t*(t * 6 - 15) + 10);
}
FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::InterpQuinticFuncDeriv(v_flt t)
{
	return 30 * t * t * (t - 1) * (t - 1);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseMath::CubicLerp(v_flt a, v_flt b, v_flt c, v_flt d, v_flt t)
{
	v_flt p = (d - c) - (a - b);
	return t * t * t * p + t * t * ((a - b) - p) + t * (c - a) + b;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH VectorRegister FVoxelFastNoiseMath::Lerp(VectorRegister a, VectorRegister b, VectorRegister t)
{
	// b - a
	VectorRegister r = VectorSubtract(b, a);
	// t * (b - a)
	r = VectorMultiply(t, r);
	// a + t * (b - a)
	r = VectorAdd(a, r);
	return r;
}

FN_FORCEINLINE_MATH VectorRegister FVoxelFastNoiseMath::InterpHermiteFunc(VectorRegister t)
{
	// 2 * t
	VectorRegister r = VectorMultiply(t, MakeVectorRegister(2.f, 2.f, 2.f, 2.f));
	// 3 - 2 * t
	r = VectorSubtract(MakeVectorRegister(3.f, 3.f, 3.f, 3.f), r);
	// t * (3 - 2 * t)
	r = VectorMultiply(r, t);
	// t * t * (3 - 2 * t)
	r = VectorMultiply(r, t);
	return r;
}

FN_FORCEINLINE_MATH VectorRegister FVoxelFastNoiseMath::InterpQuinticFunc(VectorRegister t)
{
	// t * 6
	VectorRegister r = VectorMultiply(t, MakeVectorRegister(6.f, 6.f, 6.f, 6.f));
	// t * 6 - 15
	r = VectorSubtract(r, MakeVectorRegister(15.f, 15.f, 15.f, 15.f));
	// t * (t * 6 - 15)
	r = VectorMultiply(r, t);
	// t * (t * 6 - 15) + 10
	r = VectorAdd(r, MakeVectorRegister(10.f, 10.f, 10.f, 10.f));
	// t * (t * (t * 6 - 15) + 10)
	r = VectorMultiply(r, t);
	// t * t * (t * (t * 6 - 15) + 10)
	r = VectorMultiply(r, t);
	// t * t * t * (t * (t * 6 - 15) + 10)
	r = VectorMultiply(r, t);
	return r;
}