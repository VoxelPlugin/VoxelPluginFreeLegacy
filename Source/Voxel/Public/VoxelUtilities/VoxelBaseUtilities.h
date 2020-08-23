// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Math/RandomStream.h"

namespace FVoxelUtilities
{
	template<typename T>
	FORCEINLINE constexpr bool IsPowerOfTwo(T Value)
	{
		return ((Value & (Value - 1)) == T(0));
	}
	
	FORCEINLINE constexpr int32 PositiveMod(int32 X, int32 Y)
	{
		return ((X % Y) + Y) % Y;
	}
	
	FORCEINLINE constexpr int32 DivideFloor(int32 Dividend, int32 Divisor)
	{
		int32 Q = Dividend / Divisor;
		int32 R = Dividend % Divisor;
		if ((R != 0) && ((R < 0) != (Divisor < 0)))
		{
			Q--;
		}
		return Q;
	}
	
	FORCEINLINE constexpr int32 DivideCeil(int32 Dividend, int32 Divisor)
	{
		return (Dividend > 0) ? 1 + (Dividend - 1) / Divisor : (Dividend / Divisor);
	}
	FORCEINLINE constexpr int64 DivideCeil64(int64 Dividend, int64 Divisor)
	{
		return (Dividend > 0) ? 1 + (Dividend - 1) / Divisor : (Dividend / Divisor);
	}
	
	FORCEINLINE constexpr int32 DivideRound(int32 Dividend, int32 Divisor)
	{
		const int32 R = PositiveMod(Dividend, Divisor);
		if (R < Divisor / 2)
		{
			return DivideFloor(Dividend, Divisor);
		}
		else
		{
			return DivideCeil(Dividend, Divisor);
		}
	}

	FORCEINLINE constexpr int32 IntLog2(int32 X)
	{
		int32 Exp = -1;
		while (X)
		{
			X >>= 1;
			++Exp;
		}
		return Exp;
	}

	FORCEINLINE uint8 CastToUINT8(int32 Value)
	{
		ensureMsgfVoxelSlowNoSideEffects(0 <= Value && Value < 256, TEXT("Invalid uint8 value: %d"), Value);
		return Value;
	}

	template<typename T>
	FORCEINLINE constexpr T Clamp( const T X, const T Min, const T Max )
	{
		return X < Min ? Min : X < Max ? X : Max;
	}

	FORCEINLINE constexpr int8 ClampToINT8(int32 Value)
	{
		return Clamp<int32>(Value, MIN_int8, MAX_int8);
	}
	FORCEINLINE constexpr uint8 ClampToUINT8(int32 Value)
	{
		return Clamp<int32>(Value, MIN_uint8, MAX_uint8);
	}
	FORCEINLINE constexpr int8 ClampToINT16(int32 Value)
	{
		return Clamp<int32>(Value, MIN_int16, MAX_int16);
	}
	FORCEINLINE constexpr uint16 ClampToUINT16(int32 Value)
	{
		return Clamp<int32>(Value, MIN_uint16, MAX_uint16);
	}

	FORCEINLINE uint8 FloatToUINT8(float Float)
	{
		return ClampToUINT8(FMath::FloorToInt(Float * 255.999f));
	}
	FORCEINLINE constexpr float UINT8ToFloat(uint8 Int)
	{
		return Int / 255.f;
	}
	
	// Round up if the new value is higher than the previous one, to avoid being stuck
	FORCEINLINE uint8 FloatToUINT8_ForLerp(float NewValue, float OldValue)
	{
		return ClampToUINT8(NewValue > OldValue
			? FMath::CeilToInt(NewValue * 255.999f)
			: FMath::FloorToInt(NewValue * 255.999f));
	}
	FORCEINLINE uint8 LerpUINT8(uint8 A, uint8 B, float Alpha)
	{
		const float Result = FMath::Lerp<float>(A, B, Alpha);
		// Do special rounding to not get stuck, eg Lerp(251, 255, 0.1) = 251 should be 252 instead
		// and Lerp(255, 251, 0.1) should be 254
		const int32 RoundedResult = (Alpha > 0) == (A < B) ? FMath::CeilToInt(Result) : FMath::FloorToInt(Result);
		return ClampToUINT8(RoundedResult);
	}

	FORCEINLINE FColor FloatToUINT8(FLinearColor Float)
	{
		return
		FColor
		{
			FloatToUINT8(Float.R),
			FloatToUINT8(Float.G),
			FloatToUINT8(Float.B),
			FloatToUINT8(Float.A)
		};
	}
	FORCEINLINE FLinearColor UINT8ToFloat(FColor Int)
	{
		return
		FLinearColor
		{
			UINT8ToFloat(Int.R),
			UINT8ToFloat(Int.G),
			UINT8ToFloat(Int.B),
			UINT8ToFloat(Int.A)
		};
	}
	
	FORCEINLINE uint16 FloatToUINT16(float Float)
	{
		return ClampToUINT16(FMath::FloorToInt(Float * 65535.999f));
	}
	FORCEINLINE constexpr float UINT16ToFloat(uint16 Int)
	{
		return Int / 65535.f;
	}
	
	FORCEINLINE constexpr uint32 MurmurHash32(uint32 Hash)
	{
		Hash ^= Hash >> 16;
		Hash *= 0x85ebca6b;
		Hash ^= Hash >> 13;
		Hash *= 0xc2b2ae35;
		Hash ^= Hash >> 16;
		return Hash;
	}
	FORCEINLINE uint32 MurmurHash32(int32 Hash)
	{
		return MurmurHash32(*reinterpret_cast<uint32*>(&Hash));
	}
	// Slow!
	template<typename T, typename... TArgs>
	FORCEINLINE uint32 MurmurHash32(T A, TArgs... Args)
	{
		return MurmurHash32(MurmurHash32(A) ^ MurmurHash32(Args...));
	}

	FORCEINLINE constexpr uint64 MurmurHash64(uint64 Hash)
	{
		Hash ^= Hash >> 33;
		Hash *= 0xff51afd7ed558ccd;
		Hash ^= Hash >> 33;
		Hash *= 0xc4ceb9fe1a85ec53;
		Hash ^= Hash >> 33;
		return Hash;
	}
	FORCEINLINE uint64 MurmurHash64(int64 Hash)
	{
		return MurmurHash64(*reinterpret_cast<uint64*>(&Hash));
	}
	// Slow!
	template<typename T, typename... TArgs>
	FORCEINLINE uint32 MurmurHash64(T A, TArgs... Args)
	{
		return MurmurHash32(MurmurHash64(A) ^ MurmurHash64(Args...));
	}

	FORCEINLINE uint32 MurmurHash32xN(uint32 const* RESTRICT Hash, int32 Size, uint32 Seed = 0)
	{
		uint32 H = Seed;
		for (int32 Index = 0; Index < Size; ++Index)
		{
			uint32 K = Hash[Index];
			K *= 0xcc9e2d51;
			K = (K << 15) | (K >> 17);
			K *= 0x1b873593;
			H ^= K;
			H = (H << 13) | (H >> 19);
			H = H * 5 + 0xe6546b64;
		}

		H ^= uint32(Size);
		H ^= H >> 16;
		H *= 0x85ebca6b;
		H ^= H >> 13;
		H *= 0xc2b2ae35;
		H ^= H >> 16;
		return H;
	}

	template<uint32 Base>
	FORCEINLINE float Halton(uint32 Index)
	{
		float Result = 0.0f;
		const float InvBase = 1.0f / Base;
		float Fraction = InvBase;
		while (Index > 0)
		{
			Result += (Index % Base) * Fraction;
			Index /= Base;
			Fraction *= InvBase;
		}
		return Result;
	}

	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 *  -----> X
	 */
	template<typename T, typename U = float>
	FORCEINLINE T BilinearInterpolation(T A, T B, T C, T D, U X, U Y)
	{
		T AB = FMath::Lerp<T, U>(A, B, X);
		T CD = FMath::Lerp<T, U>(C, D, X);
		return FMath::Lerp<T, U>(AB, CD, Y);
	}
	
	/**
	 * Y
	 * ^ C - D
	 * | |   |
	 * | A - B
	 * 0-----> X
	 * Y
	 * ^ G - H
	 * | |   |
	 * | E - F
	 * 1-----> X
	 */
	template<typename T, typename U = float>
	FORCEINLINE T TrilinearInterpolation(
		T A, T B, T C, T D,
		T E, T F, T G, T H,
		U X, U Y, U Z)
	{
		const T ABCD = BilinearInterpolation<T, U>(A, B, C, D, X, Y);
		const T EFGH = BilinearInterpolation<T, U>(E, F, G, H, X, Y);
		return FMath::Lerp<T, U>(ABCD, EFGH, Z);
	}

	template<typename T>
	FORCEINLINE T&& VariadicMin(T&& Val)
	{
		return Forward<T>(Val);
	}
	template<typename T0, typename T1, typename... Ts>
	FORCEINLINE auto VariadicMin(T0&& Val1, T1&& Val2, Ts&&... Vals)
	{
		return (Val1 < Val2) ?
			VariadicMin(Val1, Forward<Ts>(Vals)...) :
			VariadicMin(Val2, Forward<Ts>(Vals)...);
	}

	template<typename T>
	FORCEINLINE T&& VariadicMax(T&& Val)
	{
		return Forward<T>(Val);
	}
	template<typename T0, typename T1, typename... Ts>
	FORCEINLINE auto VariadicMax(T0&& Val1, T1&& Val2, Ts&&... Vals)
	{
		return (Val1 > Val2) ?
			VariadicMax(Val1, Forward<Ts>(Vals)...) :
			VariadicMax(Val2, Forward<Ts>(Vals)...);
	}

	FORCEINLINE uint32 Popc(uint32 Value)
	{
		// Java: use >>> instead of >>
		// C or C++: use uint32_t
		Value = Value - ((Value >> 1) & 0x55555555);
		Value = (Value & 0x33333333) + ((Value >> 2) & 0x33333333);
		return (((Value + (Value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
	}

	// Returns distance to voxel with Density
	FORCEINLINE float GetAbsDistanceFromDensities(float Density, float OtherDensity)
	{
		ensureVoxelSlowNoSideEffects(Density > 0 != OtherDensity > 0);
		return Density / (Density - OtherDensity);
	}
}