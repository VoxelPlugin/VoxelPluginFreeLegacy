// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelContainers/VoxelStaticArray.h"

template<int32 NumChannels>
struct TVoxelInterpolator
{
	TVoxelStaticArray<float, NumChannels> Data;

#define OP(bop, op) \
	FORCEINLINE TVoxelInterpolator<NumChannels>& operator op(const TVoxelInterpolator<NumChannels>& Other) \
	{ \
		for (int32 Index = 0; Index < NumChannels; Index++) \
		{ \
			Data[Index] op Other.Data[Index]; \
		} \
		return *this; \
	} \
	FORCEINLINE TVoxelInterpolator<NumChannels> operator bop(const TVoxelInterpolator<NumChannels>& Other) const \
	{ \
		auto Copy = *this; \
		return Copy op Other; \
	}

	OP(+, +=)
	OP(-, -=)
	OP(*, *=)
	OP(/, /=)

#undef OP

	FORCEINLINE TVoxelInterpolator<NumChannels>& operator*=(float Other)
	{
		for (int32 Index = 0; Index < NumChannels; Index++)
		{
			Data[Index] *= Other;
		}
		return *this;
	}
	FORCEINLINE TVoxelInterpolator<NumChannels> operator*(float Other) const
	{
		auto Copy = *this;
		return Copy *= Other;
	}
	FORCEINLINE friend TVoxelInterpolator<NumChannels> operator*(float Other, const TVoxelInterpolator<NumChannels>& This)
	{
		return This * Other;
	}
};