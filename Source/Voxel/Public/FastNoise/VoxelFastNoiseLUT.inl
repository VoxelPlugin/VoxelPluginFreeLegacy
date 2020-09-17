// Copyright 2020 Phyronnaz

#pragma once

#include "FastNoise/VoxelFastNoiseLUT.h"
#include "FastNoise/VoxelFastNoiseMath.inl"

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

FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index2D_12(uint8 offset, int32 x, int32 y) const
{
	return Perm12[(x & 0xff) + Perm[(y & 0xff) + offset]];
}
FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index3D_12(uint8 offset, int32 x, int32 y, int32 z) const
{
	return Perm12[(x & 0xff) + Perm[(y & 0xff) + Perm[(z & 0xff) + offset]]];
}
FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index4D_32(uint8 offset, int32 x, int32 y, int32 z, int32 w) const
{
	return Perm[(x & 0xff) + Perm[(y & 0xff) + Perm[(z & 0xff) + Perm[(w & 0xff) + offset]]]] & 31;
}
FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index2D_256(uint8 offset, int32 x, int32 y) const
{
	return Perm[(x & 0xff) + Perm[(y & 0xff) + offset]];
}
FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index3D_256(uint8 offset, int32 x, int32 y, int32 z) const
{
	return Perm[(x & 0xff) + Perm[(y & 0xff) + Perm[(z & 0xff) + offset]]];
}
FN_FORCEINLINE_MATH uint8 FVoxelFastNoiseLUT::Index4D_256(uint8 offset, int32 x, int32 y, int32 z, int32 w) const
{
	return Perm[(x & 0xff) + Perm[(y & 0xff) + Perm[(z & 0xff) + Perm[(w & 0xff) + offset]]]];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::ValCoord2DFast(uint8 offset, int32 x, int32 y) const
{
	return VAL_LUT[Index2D_256(offset, x, y)];
}
FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::ValCoord3DFast(uint8 offset, int32 x, int32 y, int32 z) const
{
	return VAL_LUT[Index3D_256(offset, x, y, z)];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::GradCoord2D(uint8 offset, int32 x, int32 y, v_flt xd, v_flt yd) const
{
	const uint8 lutPos = Index2D_12(offset, x, y);
	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos];
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::GradCoord2D(uint8 offset, int32 x, int32 y, v_flt xd, v_flt yd, v_flt& outGradX, v_flt& outGradY) const
{
	uint8 lutPos = Index2D_12(offset, x, y);

	outGradX = GRAD_X[lutPos];
	outGradY = GRAD_Y[lutPos];

	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos];
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::GradCoord3D(uint8 offset, int32 x, int32 y, int32 z, v_flt xd, v_flt yd, v_flt zd) const
{
	uint8 lutPos = Index3D_12(offset, x, y, z);

	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos] + zd * GRAD_Z[lutPos];
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::GradCoord3D(uint8 offset, int32 x, int32 y, int32 z, v_flt xd, v_flt yd, v_flt zd, v_flt& outGradX, v_flt& outGradY, v_flt& outGradZ) const
{
	uint8 lutPos = Index3D_12(offset, x, y, z);
	
	outGradX = GRAD_X[lutPos];
	outGradY = GRAD_Y[lutPos];
	outGradZ = GRAD_Z[lutPos];

	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos] + zd * GRAD_Z[lutPos];
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::GradCoord4D(uint8 offset, int32 x, int32 y, int32 z, int32 w, v_flt xd, v_flt yd, v_flt zd, v_flt wd) const
{
	uint8 lutPos = Index4D_32(offset, x, y, z, w) << 2;

	return xd * GRAD_4D[lutPos] + yd * GRAD_4D[lutPos + 1] + zd * GRAD_4D[lutPos + 2] + wd * GRAD_4D[lutPos + 3];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH VectorRegister FVoxelFastNoiseLUT::ValCoord2DFast(VectorRegisterInt offset, VectorRegisterInt x, VectorRegisterInt y) const
{
	x = VectorIntAnd(x, MakeVectorRegisterInt(0xFF, 0xFF, 0xFF, 0xFF));
	y = VectorIntAnd(y, MakeVectorRegisterInt(0xFF, 0xFF, 0xFF, 0xFF));

	y = VectorIntAdd(y, offset);
	
	int32 xv[4];
	int32 yv[4];
	VectorIntStore(x, xv);
	VectorIntStore(y, yv);

	float Result[4];
	Result[0] = VAL_LUT[Perm[xv[0] + Perm[yv[0]]]];
	Result[1] = VAL_LUT[Perm[xv[1] + Perm[yv[1]]]];
	Result[2] = VAL_LUT[Perm[xv[2] + Perm[yv[2]]]];
	Result[3] = VAL_LUT[Perm[xv[3] + Perm[yv[3]]]];

	return VectorLoad(Result);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::ValCoord2D(int32 seed, int32 x, int32 y)
{
	int32 n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;

	return (n * n * n * 60493) / v_flt(2147483648);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::ValCoord3D(int32 seed, int32 x, int32 y, int32 z)
{
	int32 n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;
	n ^= Z_PRIME * z;

	return (n * n * n * 60493) / v_flt(2147483648);
}

FN_FORCEINLINE_MATH v_flt FVoxelFastNoiseLUT::ValCoord4D(int32 seed, int32 x, int32 y, int32 z, int32 w)
{
	int32 n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;
	n ^= Z_PRIME * z;
	n ^= W_PRIME * w;

	return (n * n * n * 60493) / v_flt(2147483648);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FN_FORCEINLINE_MATH VectorRegister FVoxelFastNoiseLUT::ValCoord2D(VectorRegisterInt seed, VectorRegisterInt x, VectorRegisterInt y)
{
	VectorRegisterInt hash = seed;

	// Note: can be removed
	x = VectorIntMultiply(x, MakeVectorRegisterInt(X_PRIME, X_PRIME, X_PRIME, X_PRIME));
	y = VectorIntMultiply(y, MakeVectorRegisterInt(Y_PRIME, Y_PRIME, Y_PRIME, Y_PRIME));

	hash = VectorIntXor(x, hash);
	hash = VectorIntXor(y, hash);

	hash = VectorIntMultiply(VectorIntMultiply(VectorIntMultiply(hash, hash), MakeVectorRegisterInt(60493, 60493, 60493, 60493)), hash);

	return VectorMultiply(MakeVectorRegister(1.f / 2147483648.f, 1.f / 2147483648.f, 1.f / 2147483648.f, 1.f / 2147483648.f), VectorIntToFloat(hash));
}
