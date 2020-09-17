// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoise_WhiteNoise.h"
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
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoise_2D(v_flt x, v_flt y) const
{
	return This().ValCoord2D(This().Seed,
		*reinterpret_cast<int32*>(&x) ^ (*reinterpret_cast<int32*>(&x) >> 16),
		*reinterpret_cast<int32*>(&y) ^ (*reinterpret_cast<int32*>(&y) >> 16));
}

template<typename T>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoiseInt_2D(int32 x, int32 y) const
{
	return This().ValCoord2D(This().Seed, x, y);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoise_3D(v_flt x, v_flt y, v_flt z) const
{
	return This().ValCoord3D(This().Seed,
		*reinterpret_cast<int32*>(&x) ^ (*reinterpret_cast<int32*>(&x) >> 16),
		*reinterpret_cast<int32*>(&y) ^ (*reinterpret_cast<int32*>(&y) >> 16),
		*reinterpret_cast<int32*>(&z) ^ (*reinterpret_cast<int32*>(&z) >> 16));
}

template<typename T>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoiseInt_3D(int32 x, int32 y, int32 z) const
{
	return This().ValCoord3D(This().Seed, x, y, z);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoise_4D(v_flt x, v_flt y, v_flt z, v_flt w) const
{
	return This().ValCoord4D(This().Seed,
		*reinterpret_cast<int32*>(&x) ^ (*reinterpret_cast<int32*>(&x) >> 16),
		*reinterpret_cast<int32*>(&y) ^ (*reinterpret_cast<int32*>(&y) >> 16),
		*reinterpret_cast<int32*>(&z) ^ (*reinterpret_cast<int32*>(&z) >> 16),
		*reinterpret_cast<int32*>(&w) ^ (*reinterpret_cast<int32*>(&w) >> 16));
}

template<typename T>
FN_FORCEINLINE_MATH v_flt TVoxelFastNoise_WhiteNoise<T>::GetWhiteNoiseInt_4D(int32 x, int32 y, int32 z, int32 w) const
{
	return This().ValCoord4D(This().Seed, x, y, z, w);
}