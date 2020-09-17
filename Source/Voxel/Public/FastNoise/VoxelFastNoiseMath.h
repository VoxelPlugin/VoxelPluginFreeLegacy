// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

// Slower when inlining everything
#define FN_FORCEINLINE inline
#define FN_FORCEINLINE_MATH FORCEINLINE
// 5% faster for fractals
#define FN_FORCEINLINE_SINGLE FORCEINLINE

class FVoxelFastNoiseMath
{
public:
	static int32 FastFloor(v_flt f);
	static int32 FastRound(v_flt f);
	static int32 FastAbs(int32 i);
	static v_flt FastAbs(v_flt f);
	static v_flt FastAbsDeriv(v_flt x, v_flt dx);
	static v_flt Lerp(v_flt a, v_flt b, v_flt t);
	static v_flt InterpHermiteFunc(v_flt t);
	static v_flt InterpHermiteFuncDeriv(v_flt t);
	static v_flt InterpQuinticFunc(v_flt t);
	static v_flt InterpQuinticFuncDeriv(v_flt t);
	static v_flt CubicLerp(v_flt a, v_flt b, v_flt c, v_flt d, v_flt t);

public:
	static VectorRegister Lerp(VectorRegister a, VectorRegister b, VectorRegister t);
	static VectorRegister InterpHermiteFunc(VectorRegister t);
	static VectorRegister InterpQuinticFunc(VectorRegister t);
};